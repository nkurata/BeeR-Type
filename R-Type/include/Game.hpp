#ifndef GAME_HPP
#define GAME_HPP

#include "AGame.hpp"
#include "Registry.hpp"
#include "Players.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"
#include "Boss.hpp"
#include "Server.hpp"
#include <random>

class Game : public AGame {
public:
    Game(Server* server);

    void registerComponents() override;
    void processPlayerActions() override;
    void initializeplayers(int numPlayers);
    void update() override;
    void handlePlayerMove(int playerId, int actionId);
    void run(int numPlayers);

    void spawnEnemy(int enemyId, float x, float y);
    void spawnBoss(int boosId, float x, float y);
    void spawnPlayer(int playerId, float x, float y);
    void spawnBullet(int playerId);
    void killBosses(int entityId);
    void killBullets(int entityId);
    void killEnemies(int entityId);
    void killPlayers(int entityId);
    void killEntity(int entityId);

    std::pair<float, float> getPlayerPosition(int playerId) const;


    bool isBossSpawned() const;
    bool areEnemiesCleared() const;
    void startNextWave();
    void spawnEnemiesRandomly();

    size_t getPlayerCount() const;
    size_t getEnemiesCount() const;
    size_t getBulletsCount() const;
    size_t getBossCount() const;

    bool hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions);
    void playerPacketFactory();
    void enemyPacketFactory();
    void bulletPacketFactory();
    void bossPacketFactory();
    void PacketFactory();
    Game* *getGame(Server *server);
    Registry::Entity getEntityFromMap(int mapId, std::string type);

private:

    std::unordered_map<int, Player> players;
    std::unordered_map<int, Enemy> enemies;
    std::unordered_map<int, Bullet> bullets;
    std::unordered_map<int, Boss> bosses;

    Server* server_;
    Registry registry;
    std::mt19937 rng;
    std::uniform_real_distribution<float> distX;
    std::uniform_real_distribution<float> distY;
    std::uniform_int_distribution<int> distTime;
    int currentWave;
    int enemiesPerWave;
    std::chrono::steady_clock::time_point lastSpawnTime;
    int nextEnemyId;
    int nextBossId;
};

#endif // GAME_HPP
