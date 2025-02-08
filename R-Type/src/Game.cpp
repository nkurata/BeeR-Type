#include "Game.hpp"
#include "Server.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include "Projectile.hpp"
#include "PositionSystem.hpp"
#include "CollisionSystem.hpp"
#include "ProjectileSystem.hpp"
#include <iostream>
#include <thread>

Game::Game(Server* server)
    : AGame(server), rng(std::random_device()()), distX(0.0f, 800.0f), distY(0.0f, 600.0f),
      distTime(1000, 5000), currentWave(0), enemiesPerWave(5), nextEnemyId(0), nextBossId(0), server_(server) {
    registry.register_component<Position>();
    registry.register_component<Velocity>();
    registry.register_component<Controllable>();
    registry.register_component<Collidable>();
    registry.register_component<Projectile>();

    registry.add_system<Position, Velocity>(positionSystem);
    registry.add_system<Position, Velocity, Projectile, Collidable>(projectileSystem);
    }

void Game::update() {
    registry.run_systems();
    processPlayerActions();
}


void Game::processPlayerActions() {
    for (auto& action : playerActions_) {
        int playerId = action.getId();
        int actionId = action.getActionId();

        if (actionId >= static_cast<int>(Network::PacketType::PLAYER_UP) && actionId <= static_cast<int>(Network::PacketType::PLAYER_STOP)) {
            handlePlayerMove(playerId, actionId);
            action.setProcessed(true);
            playerPacketFactory();
        } else if (actionId == static_cast<int>(Network::PacketType::PLAYER_SHOOT) || actionId == static_cast<int>(Network::PacketType::PLAYER_BLAST)) {
            action.setProcessed(true);
            bulletPacketFactory();
        }
        // Handle other actions or ignore unknown action IDs
    }
    std::lock_guard<std::mutex> lock(playerActionsMutex_);
    deletePlayerAction();
}

void Game::run(int numPlayers) {
    while (true) {
        update();
    }
}

void Game::handlePlayerMove(int playerId, int actionId) {
    auto entity = getEntityFromMap(playerId, "player");
    auto& positionOpt = registry.get_components<Position>()[entity];
    std::cout << "coucou est moi" << positionOpt.value().x << std::endl;
    auto& controllableOpt = registry.get_components<Controllable>()[getEntityFromMap(playerId, "player")];
    if (!controllableOpt.has_value()) {
        throw std::runtime_error("Controllable component not found for entity");
    }
    Controllable* controllableComponent = &controllableOpt.value();

    if (actionId == static_cast<int>(Network::PacketType::PLAYER_UP)) {
        controllableComponent->moveUp = true;
    } else if (actionId == static_cast<int>(Network::PacketType::PLAYER_DOWN)) {
        controllableComponent->moveDown = true;
    } else if (actionId == static_cast<int>(Network::PacketType::PLAYER_LEFT)) {
        controllableComponent->moveLeft = true;
    } else if (actionId == static_cast<int>(Network::PacketType::PLAYER_RIGHT)) {
        controllableComponent->moveRight = true;
    } else if (actionId == static_cast<int>(Network::PacketType::PLAYER_STOP)) {
        controllableComponent->moveUp = false;
        controllableComponent->moveDown = false;
        controllableComponent->moveLeft = false;
        controllableComponent->moveRight = false;
    }
}


// Player functions
void Game::spawnPlayer(int playerId, float x, float y) {
    players.emplace(playerId, new Player(registry, x, y));

    std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Player " << playerId << " spawned at: " << x << ", " << y << std::endl;

    auto packet = server_->createPacket(Network::PacketType::PLAYER_CREATE, data);
    server_->Broadcast(packet);
}

void Game::killPlayers(int entityId) {
    for (auto it = players.begin(); it != players.end();) {
        if (it->second->getEntity() == entityId) {
            registry.kill_entity(it->second->getEntity());
            it = players.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::KILL, data));
            break;
        } else {
            ++it;
        }
    }
}

// Enemy functions
void Game::spawnEnemy(int enemyId, float x, float y) {
    enemies.emplace(enemyId, new Enemy(registry, x, y));

    std::string data = std::to_string(enemyId) + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Enemy " << enemyId << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::ENEMY_CREATE, data));
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

    bullets.emplace(nextBulletId++, new Bullet(registry, positionComponent->x, positionComponent->y, velocityComponent->vx));

    std::string data = std::to_string(nextBulletId) + ";" + std::to_string(positionComponent->x) + ";" + std::to_string(positionComponent->y);
    std::cout << "Bullet " << nextBulletId << " spawned at: " << positionComponent->x << ", " << positionComponent->y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::BULLET_CREATE, data));
}

void Game::killBullets(int entityId) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        if (it->second->getEntity() == entityId) {
            registry.kill_entity(it->second->getEntity());
            it = bullets.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::KILL, data));
            break;
        } else {
            ++it;
        }
    }
}

// Boss functions
void Game::spawnBoss(int bossId, float x, float y) {
    bosses.emplace(bossId, new Boss(registry, x, y));

    std::string data = std::to_string(bossId) + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Boss " << bossId << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::BOSS_CREATE, data));
}

void Game::killBosses(int entityId) {
    for (auto it = bosses.begin(); it != bosses.end();) {
        if (it->second->getEntity() == entityId) {
            registry.kill_entity(it->second->getEntity());
            it = bosses.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::KILL, data));
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
        return players.at(mapId)->getEntity();
    } else if (type == "enemy") {
        return enemies.at(mapId)->getEntity();
    } else if (type == "bullet") {
        return bullets.at(mapId)->getEntity();
    } else if (type == "boss") {
        return bosses.at(mapId)->getEntity();
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