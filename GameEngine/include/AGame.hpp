#ifndef AGAME_HPP
#define AGAME_HPP

#include "IGame.hpp"
#include "Players.hpp"
#include "Bullet.hpp"
#include "Registry.hpp"
#include "PlayerAction.hpp"
#include "Position.hpp"
#include "Drawable.hpp"
#include "Collidable.hpp"
#include "Controllable.hpp"
#include "Projectile.hpp"
#include "Velocity.hpp"
#include "Registry.hpp"
#include <vector>
#include <mutex>
#include <map>

class Server;

class AGame : public IGame {
    public:
        AGame(Server* server);
        virtual ~AGame();

        // Implement player action management functions
        void addPlayerAction(int playerId, int actionId) override;
        void processPlayerActions() override;
        void deletePlayerAction() override;
        const std::vector<PlayerAction>& getPlayerActions() const override;

        //Getter functions for player positions for server to build package to send to client
        std::pair<float, float> getPlayerPosition(int playerId) const override;
        std::pair<float, float> getBulletPosition(int bulletId) const override;

        // Implement entity spawn and delete management functions
        void spawnPlayer(int playerId, float x, float y) override;
        void spawnBullet(int playerId) override;
        void killPlayers(int entityId) override;
        void killBullets(int entityId) override;
        void killEntity(int entityId) override;

        void registerComponents();
    protected:
        std::vector<PlayerAction> playerActions;
        std::map<int, Player> players;
        std::map<int, Bullet> bullets;
        std::unordered_map<int, int> bulletIdWhoShot;
        std::unordered_map<int, int> bulletIdWhoShotFast;
        Registry registry;
        Server* m_server;
        std::mutex playerActionsMutex;
};

#endif // AGAME_HPP
