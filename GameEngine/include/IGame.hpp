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

    // Registry functions
    virtual void registerComponents();

    // Functions for managing player actions
    virtual void addPlayerAction(int playerId, int actionId) = 0;
    virtual void processPlayerActions() = 0;
    virtual void deletePlayerAction() = 0;
    virtual const std::vector<PlayerAction>& getPlayerActions() const = 0;
};

#endif // IGAME_HPP
