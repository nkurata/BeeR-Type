#include "Game.hpp"
#include "Server.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include "Projectile.hpp"
#include <iostream>
#include <thread>

Game::Game(Server* server)
    : AGame(server), rng(std::random_device()()), distX(0.0f, 800.0f), distY(0.0f, 600.0f),
      distTime(1000, 5000), currentWave(0), enemiesPerWave(5), nextEnemyId(0), nextBossId(0), server_(server) {
    registerComponents();
    run(server->getClients().size());
    }

void Game::initializeplayers(int numPlayers) {
    for (int i = 0; i < numPlayers; ++i) {
        spawnPlayer(i, 100.0f * (i + 1.0f), 100.0f);
    }
}

void Game::update() {
    registry.run_systems();
    processPlayerActions();
}


void Game::processPlayerActions() {
    for (auto& action : playerActions_) {
        int playerId = action.getId();
        int actionId = action.getActionId();

        if (actionId > 39 && actionId < 48) { // Change by real action ID defined in server
            handlePlayerMove(playerId, actionId);
            action.setProcessed(true);
            playerPacketFactory();
        } else if (actionId == 5) { // Change by real action ID defined in server
            action.setProcessed(true);
            bulletPacketFactory();
        }
        // Handle other actions or ignore unknown action IDs
    }
    std::lock_guard<std::mutex> lock(playerActionsMutex_);
    deletePlayerAction();
}

void Game::run(int numPlayers) {
    initializeplayers(numPlayers);
    sf::RenderWindow window(sf::VideoMode(800, 600), "R-Type Game");
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        update();

        window.clear();
        window.display();
    }
}

void Game::handlePlayerMove(int playerId, int actionId) {
    if (actionId > 34 && actionId < 39) {
        //handleplayer movement
    } else if (actionId > 38 && actionId < 43) {
        //handleplayer stopping
    }
}

//Registry functions
void Game::registerComponents()
{
    registry.register_component<Position>();
    registry.register_component<Velocity>();
    registry.register_component<Drawable>();
    registry.register_component<Controllable>();
    registry.register_component<Collidable>();
    registry.register_component<Projectile>();
}


// Player functions
void Game::spawnPlayer(int playerId, float x, float y) {
    players.emplace(playerId, Player(registry, x, y));

    std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Player " << playerId << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_PLAYER, data));
}

void Game::killPlayers(int entityId) {
    for (auto it = players.begin(); it != players.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            it = players.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DELETE, data));
            break;
        } else {
            ++it;
        }
    }
}

// Enemy functions
void Game::spawnEnemy(int enemyId, float x, float y) {
    enemies.emplace(enemyId, Enemy(registry, x, y));

    std::string data = std::to_string(enemyId) + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Enemy " << enemyId << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_ENEMY, data));
}

// Bullet functions
void Game::spawnBullet(int playerId) {
    auto& positionOpt = registry.get_components<Position>()[getEntityFromMap(playerId, "player")];
    if (!positionOpt.has_value()) {
        throw std::runtime_error("Position component not found for entity");
    }
    const Position* positionComponent = &positionOpt.value();
    auto& velocityOpt = registry.get_components<Velocity>()[getEntityFromMap(playerId, "player")];
    if (!velocityOpt.has_value()) {
        throw std::runtime_error("Velocity component not found for entity");
    }
    const Velocity* velocityComponent = &velocityOpt.value();

    bullets.emplace(nextBulletId++, Bullet(registry, positionComponent->x, positionComponent->y, velocityComponent->vx));

    std::string data = std::to_string(nextBulletId) + ";" + std::to_string(positionComponent->x) + ";" + std::to_string(positionComponent->y);
    std::cout << "Bullet " << nextBulletId << " spawned at: " << positionComponent->x << ", " << positionComponent->y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_BULLET, data));
}

void Game::killBullets(int entityId) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            it = bullets.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DELETE, data));
            break;
        } else {
            ++it;
        }
    }
}

// Boss functions
void Game::spawnBoss(int bossId, float x, float y) {
    bosses.emplace(bossId, Boss(registry, x, y));

    std::string data = std::to_string(bossId) + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Boss " << bossId << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_BOSS, data));
}

void Game::killBosses(int entityId) {
    for (auto it = bosses.begin(); it != bosses.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            it = bosses.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DELETE, data));
            break;
        } else {
            ++it;
        }
    }
}


bool Game::isBossSpawned() const {
    return !bosses.empty();
}

bool Game::areEnemiesCleared() const {
    return enemies.empty();
}

void Game::startNextWave() {
    currentWave++;
    enemiesPerWave += 5; // Increase the number of enemies per wave
    for (int i = 0; i < enemiesPerWave; ++i) {
        spawnEnemy(i, distX(rng), distY(rng));
    }
}

void Game::spawnEnemiesRandomly() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSpawnTime).count();

    if (elapsed > distTime(rng) && enemies.size() < 10) {
        float x = distX(rng);
        float y = distY(rng);
        spawnEnemy(nextEnemyId++, x, y);
        lastSpawnTime = now;
    }
}

size_t Game::getPlayerCount() const {
    return players.size();
}

size_t Game::getEnemiesCount() const {
    return enemies.size();
}

size_t Game::getBulletsCount() const {
    return bullets.size();
}

size_t Game::getBossCount() const {
    return bosses.size();
}

bool Game::hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions) {
    auto it = lastKnownPositions.find(id);
    if (it == lastKnownPositions.end() || it->second != std::make_pair(x, y)) {
        lastKnownPositions[id] = {x, y};
        return true;
    }
    return false;
}

void Game::playerPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int playerId = 0; playerId < getPlayerCount(); ++playerId) {
        try {
            const auto& positionOpt = registry.get_components<Position>()[getEntityFromMap(playerId, "player")];
            if (!positionOpt.has_value()) {
                throw std::runtime_error("Position component not found for entity");
            }
            const Position* positionComponent = &positionOpt.value();
            if (hasPositionChanged(playerId, positionComponent->x, positionComponent->y, lastKnownPositions)) {
                std::string data = std::to_string(playerId) + ";" + std::to_string(positionComponent->x) + ";" + std::to_string(positionComponent->y);
                server_->Broadcast(server_->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid player ID: " << playerId << " - " << e.what() << std::endl;
        }
    }
}

void Game::enemyPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int enemyId = 0; enemyId < getEnemiesCount(); ++enemyId) {
        try {
            const auto& positionOpt = registry.get_components<Position>()[getEntityFromMap(enemyId, "enemy")];
            if (!positionOpt.has_value()) {
                throw std::runtime_error("Position component not found for entity");
            }
            const Position* positionComponent = &positionOpt.value();
            if (hasPositionChanged(enemyId, positionComponent->x, positionComponent->y, lastKnownPositions)) {
                std::string data = std::to_string(enemyId + 500) + ";" + std::to_string(positionComponent->x) + ";" + std::to_string(positionComponent->y);
                server_->Broadcast(server_->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid enemy ID: " << enemyId << " - " << e.what() << std::endl;
        }
    }
}

void Game::bulletPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int bulletId = 0; bulletId < getBulletsCount(); ++bulletId) {
        try {
            const auto& positionOpt = registry.get_components<Position>()[getEntityFromMap(bulletId, "bullet")];
            if (!positionOpt.has_value()) {
                throw std::runtime_error("Position component not found for entity");
            }
            const Position* positionComponent = &positionOpt.value();
            if (hasPositionChanged(bulletId, positionComponent->x, positionComponent->y, lastKnownPositions)) {
                std::string data = std::to_string(bulletId + 200) + ";" + std::to_string(positionComponent->x) + ";" + std::to_string(positionComponent->y);
                server_->Broadcast(server_->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid bullet ID: " << bulletId << " - " << e.what() << std::endl;
        }
    }
}

void Game::bossPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int bossId = 0; bossId < getBossCount(); ++bossId) {
        try {
            const auto& positionOpt = registry.get_components<Position>()[getEntityFromMap(bossId, "boss")];
            if (!positionOpt.has_value()) {
                throw std::runtime_error("Position component not found for entity");
            }
            const Position* positionComponent = &positionOpt.value();
            if (hasPositionChanged(bossId, positionComponent->x, positionComponent->y, lastKnownPositions)) {
                std::string data = std::to_string(bossId + 900) + ";" + std::to_string(positionComponent->x) + ";" + std::to_string(positionComponent->y);
                server_->Broadcast(server_->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid boss ID: " << bossId << " - " << e.what() << std::endl;
        }
    }
}

Registry::Entity Game::getEntityFromMap(int mapId, std::string type) {
    if (type == "player") {
        return players.at(mapId).getEntity();
    } else if (type == "enemy") {
        return enemies.at(mapId).getEntity();
    } else if (type == "bullet") {
        return bullets.at(mapId).getEntity();
    } else if (type == "boss") {
        return bosses.at(mapId).getEntity();
    } else {
        throw std::invalid_argument("Invalid entity type");
    }

}

void Game::PacketFactory() {
    playerPacketFactory();
    enemyPacketFactory();
    bulletPacketFactory();
    bossPacketFactory();
}


extern "C" Game *createGame(Server* server) {
    return new Game(server);
}