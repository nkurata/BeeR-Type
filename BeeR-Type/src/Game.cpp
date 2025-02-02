#include "Game.hpp"
#include "Server.hpp"
#include <iostream>
#include <thread>

RTypeGame::RTypeGame(RType::Server* server)
    : AGame(server), rng(std::random_device()()), distX(0.0f, 800.0f), distY(0.0f, 600.0f),
      distTime(1000, 5000), currentWave(0), enemiesPerWave(5), nextEnemyId(0), nextBossId(0) {}

void RTypeGame::initializeplayers(int numPlayers) {
    for (int i = 0; i < numPlayers; ++i) {
        spawnPlayer(i, 100.0f * (i + 1.0f), 100.0f);
    }
}

void RTypeGame::update() {
    registry.run_systems();
    processPlayerActions();
    sf::Time elapsed = gameClock.getElapsedTime();
    updateUI(elapsed);
    sendUIUpdate();
}

void RTypeGame::updateUI(sf::Time elapsed) {
    score += 1; // Example increment, adjust as needed
    gameTime = elapsed.asSeconds();
    ui.update(score, gameTime, players);
}

void RTypeGame::sendUIUpdate() {
    std::string uiData = "Score:" + std::to_string(score) + ";Time:" + std::to_string(gameTime);
    for (const auto& player : players) {
        uiData += ";Player" + std::to_string(player.getEntity()) + "Health:" + std::to_string(player.getHealth());
    }
    m_server->Broadcast(m_server->createPacket(Network::PacketType::UI_UPDATE, uiData));
}

void RTypeGame::run(int numPlayers) {
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
        renderUI(window);
        window.display();
    }
}

void RTypeGame::renderUI(sf::RenderWindow& window) {
    ui.render(window);
}

void RTypeGame::handlePlayerMove(int playerId, int actionId) {
    float moveDistance = 1.0f;
    float x = 0.0f;
    float y = 0.0f;

    if (actionId == 1) { // Left
        x = -moveDistance;
    } else if (actionId == 2) { // Right
        x = moveDistance;
    } else if (actionId == 3) { // Up
        y = -moveDistance;
    } else if (actionId == 4) { // Down
        y = moveDistance;
    }
    players[playerId].move(x, y);
}

bool RTypeGame::isBossSpawned() const {
    return !bosses.empty();
}

bool RTypeGame::areEnemiesCleared() const {
    return enemies.empty();
}

void RTypeGame::startNextWave() {
    currentWave++;
    enemiesPerWave += 5; // Increase the number of enemies per wave
    for (int i = 0; i < enemiesPerWave; ++i) {
        spawnEnemy(i, distX(rng), distY(rng));
    }
}

void RTypeGame::spawnEnemiesRandomly() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSpawnTime).count();

    if (elapsed > distTime(rng) && enemies.size() < 10) {
        float x = distX(rng);
        float y = distY(rng);
        spawnEnemy(nextEnemyId++, x, y);
        lastSpawnTime = now;
    }
}

size_t RTypeGame::getPlayerCount() const {
    return players.size();
}

size_t RTypeGame::getEnemiesCount() const {
    return enemies.size();
}

size_t RTypeGame::getBulletsCount() const {
    return bullets.size();
}

size_t RTypeGame::getBossCount() const {
    return bosses.size();
}

bool RTypeGame::hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions) {
    auto it = lastKnownPositions.find(id);
    if (it == lastKnownPositions.end() || it->second != std::make_pair(x, y)) {
        lastKnownPositions[id] = {x, y};
        return true;
    }
    return false;
}

void RTypeGame::playerPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int playerId = 0; playerId < getPlayerCount(); ++playerId) {
        try {
            auto [x, y] = getPlayerPosition(playerId);
            if (hasPositionChanged(playerId, x, y, lastKnownPositions)) {
                std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y);
                m_server->Broadcast(m_server->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid player ID: " << playerId << " - " << e.what() << std::endl;
        }
    }
}

void RTypeGame::enemyPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int enemyId = 0; enemyId < getEnemiesCount(); ++enemyId) {
        try {
            auto [x, y] = getEnemyPosition(enemyId);
            if (hasPositionChanged(enemyId, x, y, lastKnownPositions)) {
                std::string data = std::to_string(enemyId + 500) + ";" + std::to_string(x) + ";" + std::to_string(y);
                m_server->Broadcast(m_server->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid enemy ID: " << enemyId << " - " << e.what() << std::endl;
        }
    }
}

void RTypeGame::bulletPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int bulletId = 0; bulletId < getBulletsCount(); ++bulletId) {
        try {
            auto [x, y] = getBulletPosition(bulletId);
            if (hasPositionChanged(bulletId, x, y, lastKnownPositions)) {
                std::string data = std::to_string(bulletId + 200) + ";" + std::to_string(x) + ";" + std::to_string(y);
                m_server->Broadcast(m_server->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid bullet ID: " << bulletId << " - " << e.what() << std::endl;
        }
    }
}

void RTypeGame::bossPacketFactory() {
    static std::unordered_map<int, std::pair<float, float>> lastKnownPositions;

    for (int bossId = 0; bossId < getBossCount(); ++bossId) {
        try {
            auto [x, y] = getBossPosition(bossId);
            if (hasPositionChanged(bossId, x, y, lastKnownPositions)) {
                std::string data = std::to_string(bossId + 900) + ";" + std::to_string(x) + ";" + std::to_string(y);
                m_server->Broadcast(m_server->createPacket(Network::PacketType::CHANGE, data));
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "[ERROR] Invalid boss ID: " << bossId << " - " << e.what() << std::endl;
        }
    }
}

void RTypeGame::PacketFactory() {
    playerPacketFactory();
    enemyPacketFactory();
    bulletPacketFactory();
    bossPacketFactory();
}
