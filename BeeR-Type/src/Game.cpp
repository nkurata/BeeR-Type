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
      distTime(1000, 5000), currentWave(0), enemiesPerWave(5), nextEnemyId(0), nextBossId(0), score(0), gameTime(0.0f), server_(server) {
    gameClock.restart();
    registerComponents();
    run(server->getClients().size());
    }

void Game::initializeplayers(int numPlayers) {
    for (int i = 0; i < numPlayers; ++i) {
        spawnPlayer(i, 100.0f * (i + 1.0f), 100.0f);
    }
}

void Game::update() {
    registry_.run_systems();
    processPlayerActions();
    sf::Time elapsed = gameClock.getElapsedTime();
    updateUI(elapsed);
    sendUIUpdate();
}

void Game::updateUI(sf::Time elapsed) {
    score += 1; // Example increment, adjust as needed
    gameTime = elapsed.asSeconds();
    std::vector<Player> playersVector;
    for (const auto& pair : players) {
        playersVector.push_back(pair.second);
    }
    ui.update(score, gameTime, playersVector);
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

void Game::sendUIUpdate() {
    std::string uiData = "Score:" + std::to_string(score) + ";Time:" + std::to_string(gameTime);
    for (const auto& player : players) {
        uiData += ";Player" + std::to_string(player.second.getEntity()) + "Health:" + std::to_string(player.second.getHealth());
    }
    server_->Broadcast(server_->createPacket(Network::PacketType::UI_UPDATE, uiData));
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
        renderUI(window);
        window.display();
    }
}

void Game::renderUI(sf::RenderWindow& window) {
    ui.render(window);
}

void Game::handlePlayerMove(int playerId, int actionId) {
    if (actionId > 39 && actionId < 44) {
        //handleplayer movement
    } else if (actionId > 43 && actionId < 48) {
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
    if (playerId >= 0 && playerId < 4) {
        players.emplace(playerId, Player(registry, x, y));

        std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y);
        std::cout << "Player " << playerId << " spawned at " << x << ", " << y << std::endl;
        server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_PLAYER, data));
    }
}

std::pair<float, float> Game::getPlayerPosition(int playerId) const {
    if (playerId < 0 || playerId >= players_.size()) {
        throw std::out_of_range("Invalid player ID");
    }

    const auto& positionComponent = players_.at(playerId).getRegistry().get_components<Position>().at(players_.at(playerId).getEntity());
    return {positionComponent->x, positionComponent->y};
}

void Game::killPlayers(int entityId) {
    for (auto it = players_.begin(); it != players_.end();) {
        if (it->second.getEntity() == entityId) {
            registry_.kill_entity(it->second.getEntity());
            it = players_.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DELETE, data));
            break;
        } else {
            ++it;
        }
    }
}

size_t Game::getPlayerCount() const {
    return players_.size();
}

bool Game::isBossSpawned() const {
    return !bosses_.empty();
}

bool Game::areEnemiesCleared() const {
    return enemies_.empty();
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

    if (elapsed > distTime(rng) && enemies_.size() < 10) {
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

void Game::enemyPacketFactory() {
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

void Game::bulletPacketFactory() {
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

void Game::bossPacketFactory() {
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

void Game::PacketFactory() {
    playerPacketFactory();
    enemyPacketFactory();
    bulletPacketFactory();
    bossPacketFactory();
}

extern "C" Game *createGame(RType::Server* server) {
    return new Game(server);
}