/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** AGame
*/

#ifndef AGAME_HPP
#define AGAME_HPP

#include "IGame.hpp"
#include "PlayerAction.hpp"
#include "Server.hpp"
#include <vector>
#include <mutex>

class AGame : public IGame {
protected:
    std::vector<PlayerAction> playerActions_;
    Server* server_;
    std::mutex playerActionsMutex_;
public:
    AGame(Server* server);
    virtual ~AGame();
    virtual void update() = 0;
    virtual void run(int numPlayers) = 0;

    void addPlayerAction(int playerId, int actionId) override;
    void deletePlayerAction() override;
    const std::vector<PlayerAction>& getPlayerActions() const override;

    virtual void processPlayerActions() override = 0;
};

#endif // AGAME_HPP
