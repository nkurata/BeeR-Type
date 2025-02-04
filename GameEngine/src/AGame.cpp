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


AGame::AGame(Server* server) : server_(server)
{
    registerComponents();
}

AGame::~AGame()
{
    playerActions_.clear();
    players_.clear();
}

void AGame::registerComponents()
{
    registry_.register_component<Position>();
    registry_.register_component<Velocity>();
    registry_.register_component<Drawable>();
    registry_.register_component<Controllable>();
    registry_.register_component<Collidable>();
    registry_.register_component<Projectile>();
}

void AGame::addPlayerAction(int playerId, int actionId) {
    std::cout << "Player " << playerId << " performed action " << actionId << std::endl;
    std::lock_guard<std::mutex> lock(playerActionsMutex_);
    playerActions_.emplace_back(playerId, actionId);
}

void AGame::processPlayerActions() {
    for (auto& action : playerActions_) {
        int playerId = action.getId();
        int actionId = action.getActionId();

        if (actionId > 39 && actionId < 44) { // Change by real action ID defined in server
            //game_->handlePlayerStartMove(playerId, actionId);
            action.setProcessed(true);
            //game_->playerPacketFactory();
        } else if (actionId > 43 && actionId < 48) { // Change by real action ID defined in server
            //game_->handlePlayerStopMove(playerId, actionId);
            action.setProcessed(true);
            // server_->PacketFactory();
        } else if (actionId == 5) { // Change by real action ID defined in server
            action.setProcessed(true);
            // game_->bulletPacketFactory();
        }
        // Handle other actions or ignore unknown action IDs
    }
    std::lock_guard<std::mutex> lock(playerActionsMutex_);
    deletePlayerAction();
}

void AGame::deletePlayerAction() { //deletes all process elements from the vector of player actions and resizes vector
    playerActions_.erase(
        std::remove_if(playerActions_.begin(), playerActions_.end(),
            [](const PlayerAction& action) { return action.getProcessed(); }),
        playerActions_.end());
}

const std::vector<PlayerAction>& AGame::getPlayerActions() const {
    return playerActions_;
}

std::pair<float, float> AGame::getPlayerPosition(int playerId) const {
    if (playerId < 0 || playerId >= players_.size()) {
        throw std::out_of_range("Invalid player ID");
    }

    const auto& positionComponent = players_.at(playerId).getRegistry().get_components<Position>().at(players_.at(playerId).getEntity());
    return {positionComponent->x, positionComponent->y};
}

void AGame::spawnPlayer(int playerId, float x, float y) {
    if (playerId >= 0 && playerId < 4) {
        players_.emplace(playerId, Player(registry_, x, y));

        std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y);
        std::cout << "Player " << playerId << " spawned at " << x << ", " << y << std::endl;
        server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_PLAYER, data));
    }
}

void AGame::killPlayers(int entityId) {
    for (auto it = players_.begin(); it != players_.end();) {
        if (it->second.getEntity() == entityId) {
            registry_.kill_entity(it->second.getEntity());
            it = players_.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DELETE, data));
            break;
        } else {
            ++it;
        }
    }
}

size_t AGame::getPlayerCount() const {
    return players_.size();
}

Registry& AGame::getRegistry() {
    return registry_;
}