#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "AGame.hpp"
#include "Registry.hpp"
#include "Packet.hpp"
#include "Server.hpp"
#include "Enemy.hpp"
#include "Boss.hpp"
#include <random>
#include <memory>
#include <chrono>

class Game : public AGame {
private:
    std::map<int, Enemy> enemies_;
    std::map<int, Boss> bosses_;
    std::mt19937 rng;
    std::uniform_real_distribution<float> distX;
    std::uniform_real_distribution<float> distY;
    std::uniform_int_distribution<int> distTime;
    int enemiesPerWave;
    std::chrono::steady_clock::time_point lastSpawnTime;
    Server* server_;
    int enemyId;
    int bossId;
    int totalPlayers;

    std::vector<int> deadPlayers;
    std::vector<int> deadEnemies;
    std::vector<int> deadBullets;
    std::vector<int> deadBosses;

    template <typename Entity>
    void removeEntities(std::map<int, Entity>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType);

public:
    Game(Server* server);

    void initializePlayers(int numPlayers);
    void update() override;

    void handlePlayerMove(int playerId, int actionId) override;

    void updateBullets();
    void updateEnemies();

    void run(int numPlayers);
    void stop();

    int currentWave;
    void startNextWave();

    void checkCollisions();

    const Registry& getEntityRegistry(Registry::Entity entity);

    size_t getPlayerCount() const override;
    size_t getBulletsCount() const override;
    size_t getEnemiesCount() const;
    size_t getBossCount() const;

    void updateUI();

    bool checkPlayersAlive();
    static void checkAlivePlayers(std::vector<int>& playersToKill, const Player& player, int playerId);

    void playerPacketFactory() const;

    static bool hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions);

    void spawnBoss(int bossId, float x, float y);
    void spawnEnemy(int enemyId, float x, float y);

    void killEnemies(int entityId);
    void killBosses(int entityId);

    std::pair<float, float> getEnemyPosition(int enemyId) const;
    std::pair<float, float> getBossPosition(int bossId) const;

    void waveSystem();

    std::pair<float, float> clampPositionToBounds(float x, float y) const;
    void detectOutOfBounds(std::vector<int>& playersToKill, std::vector<int>& bulletsToKill, std::vector<int>& enemiesToKill, std::vector<int>& bossesToKill);

    void removeEntities(std::map<int, Enemy>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType);
    void removeEntities(std::map<int, Boss>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType);
    void removeEntities(std::map<int, Player>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType);
    void removeEntities(std::map<int, Bullet>& entities, std::vector<int>& entitiesToKill, Network::PacketType packetType);

    void detectCollisions(std::map<int, Bullet>& entitiesA, std::map<int, Enemy>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp);
    void detectCollisions(std::map<int, Bullet>& entitiesA, std::map<int, Boss>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp);
    void detectCollisions(std::map<int, Player>& entitiesA, std::map<int, Enemy>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp);
    void detectCollisions(std::map<int, Player>& entitiesA, std::map<int, Boss>& entitiesB, std::vector<int>& entitiesAToKill, std::vector<int>& entitiesBToKill, float damageA, float damageB, bool checkHp);

    bool checkCollision(const Player& entityA, const Enemy& entityB, float minDistance);
    bool checkCollision(const Player& entityA, const Boss& entityB, float minDistance);
    bool checkCollision(const Bullet& entityA, const Enemy& entityB, float minDistance);
    bool checkCollision(const Bullet& entityA, const Boss& entityB, float minDistance);

    void handleEntityADeath(Player& entityA, int idA, std::vector<int>& entitiesAToKill, float damageA, bool checkHp);
    void handleEntityADeath(Bullet& entityA, int idA, std::vector<int>& entitiesAToKill, float damageA, bool checkHp);
    void handleEntityBDeath(Enemy& entityB, int idB, std::vector<int>& entitiesBToKill, float damageB, bool checkHp);
    void handleEntityBDeath(Boss& entityB, int idB, std::vector<int>& entitiesBToKill, float damageB, bool checkHp);

    void handleBulletPlayerCollision(int idA, int idB, std::vector<int>& entitiesBToKill);
};

#endif // GAME_STATE_HPP
