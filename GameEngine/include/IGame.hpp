/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** IGame
*/

#ifndef IGAME_HPP
#define IGAME_HPP

#include <vector>
#include <utility>
#include <cstddef>
#include "PlayerAction.hpp"

class IGame {
public:
    virtual ~IGame() = default;

    virtual void update() = 0;

    // Functions for managing player actions
    virtual void addPlayerAction(int playerId, int actionId) = 0;
    virtual void processPlayerActions() = 0;
    virtual void deletePlayerAction() = 0;
    virtual const std::vector<PlayerAction>& getPlayerActions() const = 0;

    // Functions for managing players
    virtual std::pair<float, float> getPlayerPosition(int playerId) const = 0;
    virtual void spawnPlayer(int playerId, float x, float y) = 0;
    virtual void killPlayers(int entityId) = 0;
    virtual size_t getPlayerCount() const = 0;

    const Registry& getEntityRegistry(Registry::Entity entity);
    void checkAndKillEntities(Registry::Entity entity1, Registry::Entity entity2);
    void registerComponents();
};

#endif // IGAME_HPP
