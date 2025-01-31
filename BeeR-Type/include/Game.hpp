#ifndef GAME_HPP
#define GAME_HPP

#include "AGame.hpp"
#include "UI.hpp"

class RTypeGame : public AGame {
public:
    RTypeGame(RType::Server* server);

    void initializeplayers(int numPlayers);
    void update() override;
    void handlePlayerMove(int playerId, int actionId) override;
    void run(int numPlayers) override;

    bool isBossSpawned() const;
    bool areEnemiesCleared() const;
    void startNextWave();
    void spawnEnemiesRandomly();

    size_t getPlayerCount() const override;
    size_t getEnemiesCount() const override;
    size_t getBulletsCount() const override;
    size_t getBossCount() const override;

    bool hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions);
    void playerPacketFactory();
    void enemyPacketFactory();
    void bulletPacketFactory();
    void bossPacketFactory();
    void PacketFactory();

private:
    void updateUI(sf::Time elapsed);
    void sendUIUpdate();
    void renderUI(sf::RenderWindow& window);

    std::mt19937 rng;
    std::uniform_real_distribution<float> distX;
    std::uniform_real_distribution<float> distY;
    std::uniform_int_distribution<int> distTime;
    int currentWave;
    int enemiesPerWave;
    std::chrono::steady_clock::time_point lastSpawnTime;
    float gameTime;
    float score;
    UI ui;
    int nextEnemyId;
    int nextBossId;
};

#endif // GAME_HPP
