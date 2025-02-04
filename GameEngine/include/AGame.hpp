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
#include <vector>
#include <mutex>

namespace RType {
    class Server;
}

class AGame : public IGame {
protected:
    std::vector<PlayerAction> playerActions_;
    Server* server_;
    std::mutex playerActionsMutex_;
public:
    AGame(Server* server);
    virtual ~AGame();
    virtual void update() = 0;

    // Registry functions for derived classes
    virtual void registerComponents();

    // Premade player action management functions
    void addPlayerAction(int playerId, int actionId) override;
    void deletePlayerAction() override;
    const std::vector<PlayerAction>& getPlayerActions() const override;

    // Implement teh way to process the actions in derived class
    virtual void processPlayerActions() = 0;

};

#endif // AGAME_HPP
