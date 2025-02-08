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
      distTime(1000, 5000), currentWave(0), enemiesPerWave(5), server_(server) {
    registry.register_component<Position>();
    registry.register_component<Velocity>();
    registry.register_component<Controllable>();
    registry.register_component<Collidable>();
    registry.register_component<Projectile>();

    registry.add_system<Position, Velocity>(positionSystem);
    registry.add_system<Position, Velocity, Projectile, Collidable>(projectileSystem);
    }

Game::~Game() {
    enemies.clear();
    bosses.clear();
}

void Game::update() {
    while (true) {
        registry.run_systems();
        processPlayerActions();
        //update bullet position
        //wave spawning
        //update enemy position
    }
}


void Game::processPlayerActions() {
    std::lock_guard<std::mutex> lock(server_->clients_mutex_);
    if (playerActions_.empty()) {
        return;
    }

    for (auto& action : playerActions_) {
        int playerId = action.getId();
        int actionId = action.getActionId();

        auto it = players.find(playerId);
        if (it == players.end()) {
            std::cerr << "Error: Player ID " << playerId << " not found." << std::endl;
            continue;
        }

        auto& player = it->second;
        std::pair<float, float> p;
        std::string msg;

        switch (actionId) {
            case static_cast<int>(Network::PacketType::PLAYER_UP):
                player->move(0.0f, -10.0f);
                break;
            case static_cast<int>(Network::PacketType::PLAYER_DOWN):
                player->move(0.0f, 10.0f);
                break;
            case static_cast<int>(Network::PacketType::PLAYER_LEFT):
                player->move(-10.0f, 0.0f);
                break;
            case static_cast<int>(Network::PacketType::PLAYER_RIGHT):
                player->move(10.0f, 0.0f);
                break;
            case static_cast<int>(Network::PacketType::PLAYER_STOP_U):
            case static_cast<int>(Network::PacketType::PLAYER_STOP_D):
            case static_cast<int>(Network::PacketType::PLAYER_STOP_L):
            case static_cast<int>(Network::PacketType::PLAYER_STOP_R):
                player->move(0.0f, 0.0f); // Set velocity to zero
                p = player->getPosition();
                msg = std::to_string(playerId) + ";" + std::to_string(p.first) + ";" + std::to_string(p.second);
                server_->Broadcast(server_->createPacket(static_cast<Network::PacketType>(actionId), msg));
                break;
            case static_cast<int>(Network::PacketType::PLAYER_SHOOT):
                spawnBullet(playerId);
                break;
            case static_cast<int>(Network::PacketType::PLAYER_BLAST):
                spawnBlast(playerId);
                break;
            default:
                break;
        }
    }

    playerActions_.clear();
}

void Game::run(int numPlayers) {
    for (int i = 0; i < numPlayers; ++i) {
        spawnPlayer(i, 100, (i * 100));
    }
    update();
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
void Game::spawnEnemy(float x, float y) {
    enemies.emplace_back(new Enemy(registry, x, y));

    std::string data = enemies.back()->getEntity() + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Enemy " << enemies.back()->getEntity() << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::ENEMY_CREATE, data));
}

// Bullet functions
void Game::spawnBullet(int playerId) {
    auto& player = players[playerId];
    std::string data = player->playerShoot();
    server_->Broadcast(server_->createPacket(Network::PacketType::PLAYER_SHOOT, data));
}

void Game::spawnBlast(int playerId) {
    auto& player = players[playerId];
    std::string data = player->playerBlast();
    server_->Broadcast(server_->createPacket(Network::PacketType::PLAYER_BLAST, data));
}

// Boss functions
void Game::spawnBoss(float x, float y) {
    bosses.emplace_back(new Boss(registry, x, y));

    std::string data = bosses.back()->getEntity() + ";" + std::to_string(x) + ";" + std::to_string(y);
    std::cout << "Boss " << bosses.back()->getEntity() << " spawned at: " << x << ", " << y << std::endl;
    server_->Broadcast(server_->createPacket(Network::PacketType::BOSS_CREATE, data));
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
        spawnEnemy(distX(rng), distY(rng));
    }
}

void Game::spawnEnemiesRandomly() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSpawnTime).count();

    if (elapsed > distTime(rng) && enemies.size() < 10) {
        float x = distX(rng);
        float y = distY(rng);
        spawnEnemy(x, y);
        lastSpawnTime = now;
    }
}

size_t Game::getPlayerCount() const {
    return players.size();
}

size_t Game::getEnemiesCount() const {
    return enemies.size();
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
    } else if (type == "boss") {
        return bosses.at(mapId)->getEntity();
    } else {
        throw std::invalid_argument("Invalid entity type");
    }
}

void Game::PacketFactory() {
    playerPacketFactory();
    enemyPacketFactory();
    bossPacketFactory();
}


extern "C" Game *createGame(Server* server) {
    return new Game(server);
}