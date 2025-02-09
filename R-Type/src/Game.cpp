#include "Game.hpp"
#include "Server.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include "Projectile.hpp"
#include <iostream>
#include <thread>

Game::Game(Server* server)
    : AGame(server), rng(std::random_device()()), distX(0.0f, 1280.0f), distY(0.0f, 720.0f),
      distTime(1000, 5000), currentWave(0), enemiesPerWave(5), server_(server), enemyId(0), bossId(0) {}

void Game::initializePlayers(int numPlayers) {
    totalPlayers = numPlayers;
    for (int i = 0; i < numPlayers; ++i) {
        spawnPlayer(i, 100.0f * (i + 1.0f), 100.0f);
    }
}

void Game::update() {
    while (true) {
        processPlayerActions();
        updateBullets();
        updateEnemies();
        checkCollisions();
        waveSystem();
        updateUI();
    }
}

void Game::run(int numPlayers) {
    registry.run_systems();
    initializePlayers(numPlayers);
    while (true) {
        update();
        if (!checkPlayersAlive()) {
            break;
        }
    }
}

void Game::stop() {
    std::lock_guard<std::mutex> lock(playerActionsMutex);
    players.clear();
    enemies_.clear();
    bullets.clear();
    bosses_.clear();
    deadPlayers.clear();
    deadEnemies.clear();
    deadBullets.clear();
    deadBosses.clear();
}

const Registry& Game::getEntityRegistry(Registry::Entity entity) {
    auto playerIt = std::find_if(players.begin(), players.end(), [entity](const auto& p) { return p.second.getEntity() == entity; });
    if (playerIt != players.end()) return playerIt->second.getRegistry();

    auto enemyIt = std::find_if(enemies_.begin(), enemies_.end(), [entity](const auto& e) { return e.second.getEntity() == entity; });
    if (enemyIt != enemies_.end()) return enemyIt->second.getRegistry();

    auto bulletIt = std::find_if(bullets.begin(), bullets.end(), [entity](const auto& b) { return b.second.getEntity() == entity; });
    if (bulletIt != bullets.end()) return bulletIt->second.getRegistry();

    auto bossIt = std::find_if(bosses_.begin(), bosses_.end(), [entity](const auto& b) { return b.second.getEntity() == entity; });
    if (bossIt != bosses_.end()) return bossIt->second.getRegistry();

    std::cerr << "Error: Entity not found in any registry.";
    throw std::runtime_error("Entity not found in any registry."); //avoid compilation warning even though it will never be reached
}

void Game::waveSystem() {
    static auto lastWaveTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastWaveTime).count();

    if (elapsed >= 10) {
        currentWave++;
        if (currentWave % 3 == 0) {
            spawnBoss(bossId++, distX(rng), distY(rng));
            for (int i = 0; i < 4; ++i) {
                spawnEnemy(enemyId++, distX(rng), distY(rng));
            }
        } else {
            startNextWave();
        }
        lastWaveTime = now;
    }
}

//Ui

void Game::updateUI() {
    static auto lastBroadcastTime = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastBroadcastTime).count();

    if (elapsed >= 1) {
        for (auto& [id, player] : players) {
            const auto& score = player.getScore();
            const auto& hp = player.getHp();
            std::string ui_data = std::to_string(id) + ";" + std::to_string(hp) + ";" + std::to_string(score) + ";0;0";

            m_server->Broadcast(m_server->createPacket(Network::PacketType::UI_UPDATE, ui_data));
        }
        lastBroadcastTime = now;
    }
}

bool Game::checkPlayersAlive() {
    totalPlayers = server_->clients_.size();
    if (totalPlayers == 0 || players.empty()) {
        std::string data = "0;0;0;0;0";
        server_->Broadcast(server_->createPacket(Network::PacketType::GAME_END, data));
        std::cout << "Game over!" << std::endl;
        stop();
        return false;
    }
    return true;
}

bool Game::hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions) {
    auto it = lastKnownPositions.find(id);
    if (it == lastKnownPositions.end() || it->second != std::make_pair(x, y)) {
        lastKnownPositions[id] = {x, y};
        return true;
    }
    return false;
}

void Game::playerPacketFactory() const {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int playerId = 0; playerId < getPlayerCount(); ++playerId) {
        try {
            auto [x, y] = getPlayerPosition(playerId);
            if (hasPositionChanged(playerId, x, y, lastKnownPositions)) {
                std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y) + ";0;0";
                server_->Broadcast(server_->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid player ID: " << playerId << " - " << e.what() << std::endl;
        }
    }
}

void Game::handlePlayerMove(int playerId, int actionId) {
    const float moveDistance = 20.0f;
    float x = 0.0f;
    float y = 0.0f;
    auto [currentX, currentY] = getPlayerPosition(playerId);
    switch (actionId) {
        case 1: // Left
            x = -moveDistance;
            break;
        case 2: // Right
            x = moveDistance;
            break;
        case 3: // Up
            y = -moveDistance;
            break;
        case 4: // Down
            y = moveDistance;
            break;
        default:
            break;
    }
    float newX = currentX + x;
    float newY = currentY + y;
    std::tie(newX, newY) = clampPositionToBounds(newX, newY);
    players[playerId].move(newX - currentX, newY - currentY);
    playerPacketFactory();
}

std::pair<float, float> Game::clampPositionToBounds(float x, float y) const {
    constexpr float screenLeft = 0.0f;
    constexpr float screenRight = 1195.0f;
    constexpr float screenTop = 0.0f;
    constexpr float screenBottom = 630.0f;

    if (x < screenLeft) {
        x = screenLeft;
    } else if (x > screenRight) {
        x = screenRight;
    }

    if (y < screenTop) {
        y = screenTop;
    } else if (y > screenBottom) {
        y = screenBottom;
    }

    return {x, y};
}

//Bullet

void Game::updateBullets() {
    static auto lastUpdateTime = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count();

    if (elapsed >= 16) {
        for (auto& [id, bullet] : bullets) {
            bullet.move(5.0f, 0.0f);
        }
        lastUpdateTime = now;
    }
}

//boss

void Game::spawnBoss(int bossId, float x, float y) {
    float spawnX = 1000.0f;
    bosses_.emplace(bossId, Boss(registry, spawnX, y));

    std::string data = std::to_string(bossId) + ";" + std::to_string(spawnX) + ";" + std::to_string(y) + ";-0.1;0";
    server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_BOSS, data));
}

void Game::killBosses(int entityId) {
    for (auto it = bosses_.begin(); it != bosses_.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DESTROY_BOSS, data));
            break;
        } else {
            ++it;
        }
    }
}

//enemy

void Game::startNextWave() {
    currentWave++;
    for (int i = 0; i < enemiesPerWave; ++i) {
        spawnEnemy(enemyId++, distX(rng), distY(rng));
    }
    enemiesPerWave += 5;
}

void Game::spawnEnemy(int enemyId, float x, float y) {
    float spawnX = 1000.0f;
    enemies_.emplace(enemyId, Enemy(registry, spawnX, y));

    std::string data = std::to_string(enemyId) + ";" + std::to_string(spawnX) + ";" + std::to_string(y) + ";-1;0";
    server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_ENEMY, data));
}

void Game::killEnemies(int entityId) {
    for (auto it = enemies_.begin(); it != enemies_.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DESTROY_ENEMY, data));
            break;
        } else {
            ++it;
        }
    }
}

void Game::updateEnemies() {
    static auto lastUpdateTime = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count();

    if (elapsed >= 16) {
        for (auto& [id, enemy] : enemies_) {
            enemy.move(-1.0f, 0.0f);
        }
        for (auto& [id, boss] : bosses_) {
            boss.move(-0.1f, 0.0f);
        }
        lastUpdateTime = now;
    }
}

void Game::checkAlivePlayers(std::vector<int>& deadPlayers, const Player& player, int playerId) {
    if (player.getHp() <= 0) {
        deadPlayers.push_back(playerId);
    }
}

//collision
void Game::checkCollisions() {
    {
        std::lock_guard<std::mutex> lock(playerActionsMutex);
        detectCollisions(bullets, enemies_, deadBullets, deadEnemies, 0.0f, 0.0f, false); // Bullet-Enemy collisions
        detectCollisions(bullets, bosses_, deadBullets, deadBosses, 0.0f, 0.0f, false); // Bullet-Boss collisions
        detectCollisions(players, enemies_, deadPlayers, deadEnemies, -10.0f, 0.0f, true); // Player-Enemy collisions
        detectCollisions(players, bosses_, deadPlayers, deadBosses, -20.0f, 0.0f, true); // Player-Boss collisions

        detectOutOfBounds(deadPlayers, deadBullets, deadEnemies, deadBosses);

        removeEntities(bullets, deadBullets, Network::PacketType::DESTROY_BULLET);
        removeEntities(enemies_, deadEnemies, Network::PacketType::DESTROY_ENEMY);
        removeEntities(players, deadPlayers, Network::PacketType::DESTROY_PLAYER);
        removeEntities(bosses_, deadBosses, Network::PacketType::DESTROY_BOSS);
    }
}

void Game::detectOutOfBounds(std::vector<int>& deadPlayers, std::vector<int>& deadBullets, std::vector<int>& deadEnemies, std::vector<int>& deadBosses) {
    for (auto& [id, bullet] : bullets) {
        const auto& positionComponent = bullet.getRegistry().get_components<Position>()[bullet.getEntity()];
        if (positionComponent->x < -50.0f || positionComponent->x > 1280.0f || positionComponent->y < -50.0f || positionComponent->y > 720.0f) {
            deadBullets.push_back(id);
        }
    }

    for (auto& [id, enemy] : enemies_) {
        const auto& positionComponent = enemy.getRegistry().get_components<Position>()[enemy.getEntity()];
        if (positionComponent->x < -50.0f || positionComponent->x > 1280.0f || positionComponent->y < -50.0f || positionComponent->y > 720.0f) {
            deadEnemies.push_back(id);
        }
    }

    for (auto& [id, boss] : bosses_) {
        const auto& positionComponent = boss.getRegistry().get_components<Position>()[boss.getEntity()];
        if (positionComponent->x < -50.0f || positionComponent->x > 1280.0f || positionComponent->y < -50.0f || positionComponent->y > 720.0f) {
            deadBosses.push_back(id);
        }
    }
}

void Game::removeEntities(std::map<int, Enemy>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType) {
    for (auto it = entitiesToKill.begin(); it != entitiesToKill.end(); ) {
        int entityId = *it;
        auto entityIt = entities.find(entityId);
        if (entityIt != entities.end()) {
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            m_server->Broadcast(m_server->createPacket(packetType, data));
            entities.erase(entityIt);
            it = entitiesToKill.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::removeEntities(std::map<int, Boss>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType) {
    for (auto it = entitiesToKill.begin(); it != entitiesToKill.end(); ) {
        int entityId = *it;
        auto entityIt = entities.find(entityId);
        if (entityIt != entities.end()) {
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            m_server->Broadcast(m_server->createPacket(packetType, data));
            entities.erase(entityIt);
            it = entitiesToKill.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::removeEntities(std::map<int, Player>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType) {
    for (auto it = entitiesToKill.begin(); it != entitiesToKill.end(); ) {
        int entityId = *it;
        auto entityIt = entities.find(entityId);
        if (entityIt != entities.end()) {
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            m_server->Broadcast(m_server->createPacket(packetType, data));
            entities.erase(entityIt);
            it = entitiesToKill.erase(it);
            --totalPlayers;
        } else {
            ++it;
        }
    }
}

void Game::removeEntities(std::map<int, Bullet>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType) {
    for (auto it = entitiesToKill.begin(); it != entitiesToKill.end(); ) {
        int entityId = *it;
        auto entityIt = entities.find(entityId);
        if (entityIt != entities.end()) {
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            m_server->Broadcast(m_server->createPacket(packetType, data));
            entities.erase(entityIt);
            it = entitiesToKill.erase(it);
            bulletIdWhoShot.erase(entityId);
        } else {
            ++it;
        }
    }
}

bool Game::checkCollision(const Player& entityA, const Enemy& entityB, float minDistance) {
    const auto& posA = entityA.getRegistry().get_components<Position>()[entityA.getEntity()];
    const auto& posB = entityB.getRegistry().get_components<Position>()[entityB.getEntity()];

    float widthA = 60.0f;
    float heightA = 80.0f;
    float widthB = 60.0f;
    float heightB = 80.0f;

    float distance = std::sqrt(std::pow(posA->x - posB->x, 2) + std::pow(posA->y - posB->y, 2));

    if (distance > minDistance) {
        return posA->x < posB->x + widthB &&
               posA->x + widthA > posB->x &&
               posA->y < posB->y + heightB &&
               posA->y + heightA > posB->y;
    }
    return false;
}

bool Game::checkCollision(const Player& entityA, const Boss& entityB, float minDistance) {
    const auto& posA = entityA.getRegistry().get_components<Position>()[entityA.getEntity()];
    const auto& posB = entityB.getRegistry().get_components<Position>()[entityB.getEntity()];

    float widthA = 60.0f;
    float heightA = 80.0f;
    float widthB = 60.0f;
    float heightB = 80.0f;

    float distance = std::sqrt(std::pow(posA->x - posB->x, 2) + std::pow(posA->y - posB->y, 2));

    if (distance > minDistance) {
        return posA->x < posB->x + widthB &&
               posA->x + widthA > posB->x &&
               posA->y < posB->y + heightB &&
               posA->y + heightA > posB->y;
    }
    return false;
}

bool Game::checkCollision(const Bullet& entityA, const Enemy& entityB, float minDistance) {
    const auto& posA = entityA.getRegistry().get_components<Position>()[entityA.getEntity()];
    const auto& posB = entityB.getRegistry().get_components<Position>()[entityB.getEntity()];

    float widthA = 60.0f;
    float heightA = 80.0f;
    float widthB = 60.0f;
    float heightB = 80.0f;

    float distance = std::sqrt(std::pow(posA->x - posB->x, 2) + std::pow(posA->y - posB->y, 2));

    if (distance > minDistance) {
        return posA->x < posB->x + widthB &&
               posA->x + widthA > posB->x &&
               posA->y < posB->y + heightB &&
               posA->y + heightA > posB->y;
    }
    return false;
}

bool Game::checkCollision(const Bullet& entityA, const Boss& entityB, float minDistance) {
    const auto& posA = entityA.getRegistry().get_components<Position>()[entityA.getEntity()];
    const auto& posB = entityB.getRegistry().get_components<Position>()[entityB.getEntity()];

    float widthA = 60.0f;
    float heightA = 80.0f;
    float widthB = 60.0f;
    float heightB = 80.0f;

    float distance = std::sqrt(std::pow(posA->x - posB->x, 2) + std::pow(posA->y - posB->y, 2));

    if (distance > minDistance) {
        return posA->x < posB->x + widthB &&
               posA->x + widthA > posB->x &&
               posA->y < posB->y + heightB &&
               posA->y + heightA > posB->y;
    }
    return false;
}

void Game::handleEntityADeath(Player& entityA, int idA, std::vector<int>& entitiesAToKill, float damageA, bool checkHp) {
    if (checkHp) {
        entityA.updateHp(damageA);
        if (entityA.getHp() <= 0) {
            entitiesAToKill.push_back(idA);
        }
    } else {
        entitiesAToKill.push_back(idA);
    }
}

void Game::handleEntityADeath(Bullet& entityA, int idA, std::vector<int>& entitiesAToKill, float damageA, bool checkHp) {
    entitiesAToKill.push_back(idA);
}

void Game::handleEntityBDeath(Enemy& entityB, int idB, std::vector<int>& entitiesBToKill, float damageB, bool checkHp) {
    if (checkHp) {
        entitiesBToKill.push_back(idB);
    } else {
        entitiesBToKill.push_back(idB);
    }
    for (auto& [id, player] : players) {
        player.updateScore(5.0f);
    }
}

void Game::handleEntityBDeath(Boss& entityB, int idB, std::vector<int>& entitiesBToKill, float damageB, bool checkHp) {
    if (checkHp) {
        entitiesBToKill.push_back(idB);
    } else {
        entitiesBToKill.push_back(idB);
    }
    for (auto& [id, player] : players) {
        player.updateScore(20.0f);
    }
}

void Game::handleBulletPlayerCollision(int idA, int idB, std::vector<int>& entitiesBToKill) {
    int playerId = bulletIdWhoShot[idA];
    std::cout << "Player " << playerId << " shot enemy " << idB << std::endl;
    entitiesBToKill.push_back(idB);
    players[playerId].updateScore(100.0f);
}

void Game::detectCollisions(std::map<int, Bullet>& entitiesA, std::map<int, Enemy>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp) {
    const float minDistance = 10.0f;

    for (auto& [idA, entityA] : entitiesA) {
        for (auto& [idB, entityB] : entitiesB) {
            if (checkCollision(entityA, entityB, minDistance)) {
                handleEntityADeath(entityA, idA, entitiesAToKill, damageA, checkHp);
                handleEntityBDeath(entityB, idB, entitiesBToKill, damageB, checkHp);
            }
        }
    }
}

void Game::detectCollisions(std::map<int, Bullet>& entitiesA, std::map<int, Boss>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp) {
    const float minDistance = 10.0f;

    for (auto& [idA, entityA] : entitiesA) {
        for (auto& [idB, entityB] : entitiesB) {
            if (checkCollision(entityA, entityB, minDistance)) {
                handleEntityADeath(entityA, idA, entitiesAToKill, damageA, checkHp);
                handleEntityBDeath(entityB, idB, entitiesBToKill, damageB, checkHp);
            }
        }
    }
}

void Game::detectCollisions(std::map<int, Player>& entitiesA, std::map<int, Enemy>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp) {
    const float minDistance = 10.0f;

    for (auto& [idA, entityA] : entitiesA) {
        for (auto& [idB, entityB] : entitiesB) {
            if (checkCollision(entityA, entityB, minDistance)) {
                handleEntityADeath(entityA, idA, entitiesAToKill, damageA, checkHp);
                handleEntityBDeath(entityB, idB, entitiesBToKill, damageB, checkHp);
            }
        }
    }
}

void Game::detectCollisions(std::map<int, Player>& entitiesA, std::map<int, Boss>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp) {
    const float minDistance = 10.0f;

    for (auto& [idA, entityA] : entitiesA) {
        for (auto& [idB, entityB] : entitiesB) {
            if (checkCollision(entityA, entityB, minDistance)) {
                handleEntityADeath(entityA, idA, entitiesAToKill, damageA, checkHp);
                handleEntityBDeath(entityB, idB, entitiesBToKill, damageB, checkHp);
            }
        }
    }
}

//getter

size_t Game::getPlayerCount() const {
    return players.size();
}

size_t Game::getEnemiesCount() const {
    return enemies_.size();
}

size_t Game::getBulletsCount() const {
    return bullets.size();
}

size_t Game::getBossCount() const {
    return bosses_.size();
}

std::pair<float, float> Game::getEnemyPosition(int enemyId) const {
    auto it = enemies_.find(enemyId);
    if (it == enemies_.end()) {
        throw std::out_of_range("Invalid enemy ID");
    }

    const auto& positionComponent = it->second.getRegistry().get_components<Position>()[it->second.getEntity()];
    return {positionComponent->x, positionComponent->y};
}

std::pair<float, float> Game::getBossPosition(int bossId) const {
    auto it = bosses_.find(bossId);
    if (it == bosses_.end()) {
        throw std::out_of_range("Invalid boss ID");
    }

    const auto& positionComponent = it->second.getRegistry().get_components<Position>()[it->second.getEntity()];
    return {positionComponent->x, positionComponent->y};
}

extern "C" Game *createGame(Server *server) {
    return new Game(server);
}