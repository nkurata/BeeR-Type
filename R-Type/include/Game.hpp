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
    ~Game() override;
    void update() override;

    void processPlayerActions() override;
    void initializeplayers(int numPlayers);
    void handlePlayerMove(int playerId, int actionId);
    void run(int numPlayers);

    void spawnEnemy(float x, float y);
    void spawnBoss(float x, float y);
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
    size_t getBossCount() const;

    bool hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions);
    void playerPacketFactory();
    void enemyPacketFactory();
    void bossPacketFactory();
    void PacketFactory();
    Game* *getGame(Server *server);
    Registry::Entity getEntityFromMap(int mapId, std::string type);

private:

    std::unordered_map<int, std::unique_ptr<Player>> players;
    std::unordered_map<int, std::unique_ptr<Enemy>> enemies;
    std::unordered_map<int, std::unique_ptr<Boss>> bosses;
    std::unordered_map<int, std::unique_ptr<Bullet>> bullets;

    Server* server_;
    Registry registry;
    std::mt19937 rng;
    std::uniform_real_distribution<float> distX;
    std::uniform_real_distribution<float> distY;
    std::uniform_int_distribution<int> distTime;
    int enemyId;
    int bossId;
    int bulletId;
    int currentWave;
    int enemiesPerWave;
    std::chrono::steady_clock::time_point lastSpawnTime;
};

#endif // GAME_HPP
