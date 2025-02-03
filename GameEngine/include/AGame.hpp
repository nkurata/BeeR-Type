/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** AGame
*/

#ifndef AGAME_HPP
#define AGAME_HPP

#include "IGame.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"
#include "Registry.hpp"
#include "Boss.hpp"
#include "Registry.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include "Projectile.hpp"
#include "PlayerAction.hpp"
#include <vector>
#include <mutex>

namespace RType {
    class Server;
}

class AGame : public IGame {
protected:
    std::unordered_map<int, Player> players_;
    Registry registry_;
    Server* server_;
    std::vector<PlayerAction> playerActions_;
    std::mutex playerActionsMutex_;
public:
    AGame(Server* server);
    virtual ~AGame();

    // Implement player action management functions
    void addPlayerAction(int playerId, int actionId) override;
    void processPlayerActions() override;
    void deletePlayerAction() override;
    const std::vector<PlayerAction>& getPlayerActions() const override;

    // Implement player management functions
    virtual std::pair<float, float> getPlayerPosition(int playerId) const = 0;
    virtual void spawnPlayer(int playerId, float x, float y) = 0;
    virtual void killPlayers(int entityId) = 0;
    virtual void handlePlayerStartMove(int playerId, int actionId);
    virtual void handlePlayerStopMove(int playerId, int actionId);
    virtual size_t getPlayerCount() const = 0;

    //Registery functions
    void registerComponents();
};

#endif // AGAME_HPP
