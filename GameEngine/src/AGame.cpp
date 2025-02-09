/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** AGame
*/

#include "Packet.hpp"
#include "PacketType.hpp"
#include "Server.hpp"
#include "AGame.hpp"
#include <algorithm>
#include <iostream>

AGame::AGame(Server* server) : server_(server) {}

AGame::~AGame() {
    playerActions_.clear();
}

void AGame::addPlayerAction(int playerId, int actionId) {
    std::lock_guard<std::mutex> lock(playerActionsMutex_);
    playerActions_.emplace_back(playerId, actionId);
}

void AGame::deletePlayerAction() {
    playerActions_.erase(
        std::remove_if(playerActions_.begin(), playerActions_.end(),
            [](const PlayerAction& action) { return action.getProcessed(); }),
        playerActions_.end());
}

const std::vector<PlayerAction>& AGame::getPlayerActions() const {
    return playerActions_;
}