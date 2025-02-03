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
    playerActions.emplace_back(playerId, actionId);
}

void AGame::processPlayerActions() {
    for (auto& action : playerActions) {
        int playerId = action.getId();
        int actionId = action.getActionId();

        if (actionId > 0 && actionId < 5) { // Change by real action ID defined in server
            handlePlayerStartMove(playerId, actionId);
            action.setProcessed(true);
            server_->playerPacketFactory();
        } else if (actionId > 10 && actionId < 15) { // Change by real action ID defined in server
            handlePlayerStopMove(playerId, actionId);
            action.setProcessed(true);
            server_->PacketFactory();
        } else if (actionId == 5) { // Change by real action ID defined in server
            spawnBullet(playerId);
            action.setProcessed(true);
            server_->bulletPacketFactory();
        }
        // Handle other actions or ignore unknown action IDs
    }
    std::lock_guard<std::mutex> lock(playerActionsMutex_);
    deletePlayerAction();
}

void AGame::deletePlayerAction() { //deletes all process elements from the vector of player actions and resizes vector
    playerActions.erase(
        std::remove_if(playerActions.begin(), playerActions.end(),
            [](const PlayerAction& action) { return action.getProcessed(); }),
        playerActions.end());
}

const std::vector<PlayerAction>& AGame::getPlayerActions() const {
    return playerActions;
}

std::pair<float, float> AGame::getPlayerPosition(int playerId) const {
    if (playerId < 0 || playerId >= players.size()) {
        throw std::out_of_range("Invalid player ID");
    }

    const auto& positionComponent = players[playerId].getRegistry().get_components<Position>()[players[playerId].getEntity()];
    return {positionComponent->x, positionComponent->y};
}

std::pair<float, float> AGame::getEnemyPosition(int enemyId) const {
    if (enemyId < 0 || enemyId >= enemies.size()) {
        throw std::out_of_range("Invalid enemy ID");
    }

    const auto& positionComponent = enemies[enemyId].getRegistry().get_components<Position>()[enemies[enemyId].getEntity()];
    return {positionComponent->x, positionComponent->y};
}

std::pair<float, float> AGame::getBulletPosition(int bulletId) const {
    if (bulletId < 0 || bulletId >= bullets.size()) {
        throw std::out_of_range("Invalid bullet ID");
    }

    const auto& positionComponent = bullets[bulletId].getRegistry().get_components<Position>()[bullets[bulletId].getEntity()];
    return {positionComponent->x, positionComponent->y};
}

std::pair<float, float> AGame::getBossPosition(int bossId) const {
    if (bossId < 0 || bossId >= bosses.size()) {
        throw std::out_of_range("Invalid boss ID");
    }

    const auto& positionComponent = bosses[bossId].getRegistry().get_components<Position>()[bosses[bossId].getEntity()];
    return {positionComponent->x, positionComponent->y};
}


void AGame::spawnPlayer(int playerId, float x, float y) {
    if (playerId >= 0 && playerId < 4) {
        players.emplace_back(registry, x, y);

        std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y);
        std::cout << "Player " << playerId << " spawned at " << x << ", " << y << std::endl;
        server_->Broadcast(server_->createPacket(Network::PacketType::CREATE_PLAYER, data));
    }
}

void AGame::killPlayers(int entityId) {
    for (auto it = players_.begin(); it != players_.end();) {
        if (it->getEntity() == entityId) {
            registry_.kill_entity(it->getEntity());
            it = players_.erase(it);
            std::string data = std::to_string(entityId) + ";0;0";
            server_->Broadcast(server_->createPacket(Network::PacketType::DELETE, data));
            break;
        } else {
            ++it;
        }
    }
}


void AGame::run(int numPlayers) {
    while (true) {
        // Update game state
        update();

        // Sleep for a short duration to simulate frame time
        // std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

const Registry& AGame::getEntityRegistry(Registry::Entity entity) {
    auto playerIt = std::find_if(players.begin(), players.end(), [entity](const auto& p) { return p.getEntity() == entity; });
    if (playerIt != players.end()) return playerIt->getRegistry();

    auto enemyIt = std::find_if(enemies.begin(), enemies.end(), [entity](const auto& e) { return e.getEntity() == entity; });
    if (enemyIt != enemies.end()) return enemyIt->getRegistry();

    auto bulletIt = std::find_if(bullets.begin(), bullets.end(), [entity](const auto& b) { return b.getEntity() == entity; });
    if (bulletIt != enemies.end()) return bulletIt->getRegistry();

    auto bossIt = std::find_if(bosses.begin(), bosses.end(), [entity](const auto& b) { return b.getEntity() == entity; });
    if (bossIt != enemies.end()) return bossIt->getRegistry();

    std::cerr << "Error: Entity not found in any registry.";
    throw std::runtime_error("Entity not found in any registry.");
}

void AGame::checkAndKillEntities(Registry::Entity entity1, Registry::Entity entity2) {
    const Registry& registry1 = getEntityRegistry(entity1);
    const Registry& registry2 = getEntityRegistry(entity2);

    if (registry1.has_component<Position>(entity1) && registry2.has_component<Position>(entity2)) {
        const auto& pos1 = registry1.get_components<Position>()[entity1];
        const auto& pos2 = registry2.get_components<Position>()[entity2];

        float distance = std::sqrt(std::pow(pos2->x - pos1->x, 2) + std::pow(pos2->y - pos1->y, 2));
        float collisionThreshold = 30.0f;

        if (distance < collisionThreshold) {
            killEntity(entity1);
            killEntity(entity2);
        }
    }
}

// void AGame::checkCollisions() {
//     for (const auto& [entity1, entity2] : collision_system(
//         registry,
//         registry.get_components<Position>(),
//         registry.get_components<Drawable>(),
//         registry.get_components<Collidable>(),
//         registry.get_components<Controllable>(),
//         registry.get_components<Projectile>()
//     )) {
//         bool isProjectile1 = registry.has_component<Projectile>(entity1);
//         bool isProjectile2 = registry.has_component<Projectile>(entity2);
//         bool isPlayer1 = registry.has_component<Controllable>(entity1);
//         bool isPlayer2 = registry.has_component<Controllable>(entity2);
//         bool isEnemy1 = std::find_if(enemies.begin(), enemies.end(),
//             [entity1](auto& e){ return e.getEntity() == entity1; }) != enemies.end();
//         bool isEnemy2 = std::find_if(enemies.begin(), enemies.end(),
//             [entity2](auto& e){ return e.getEntity() == entity2; }) != enemies.end();

//         // Projectile <-> Enemy
//         if (isProjectile1 && isEnemy2) {
//             registry.kill_entity(entity1);
//             registry.kill_entity(entity2);
//         } else if (isProjectile2 && isEnemy1) {
//             registry.kill_entity(entity1);
//             registry.kill_entity(entity2);
//         }

//         // Player <-> Enemy
//         if (isPlayer1 && isEnemy2) {
//             registry.kill_entity(entity1);
//         } else if (isPlayer2 && isEnemy1) {
//             registry.kill_entity(entity2);
//         }
//     }
// }
