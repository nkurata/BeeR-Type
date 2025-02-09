#include "Game.hpp"
#include "Packet.hpp"
#include "PacketType.hpp"
#include "Server.hpp"
#include "AGame.hpp"
#include <algorithm>
#include <iostream>

AGame::AGame(Server* server) : m_server(server)
{
    registerComponents();
}

AGame::~AGame()
{
    playerActions.clear();
    players.clear();
}

void AGame::registerComponents()
{
    registry.register_component<Position>();
    registry.register_component<Velocity>();
    registry.register_component<Drawable>();
    registry.register_component<Controllable>();
    registry.register_component<Collidable>();
    registry.register_component<Projectile>();
}

void AGame::addPlayerAction(int playerId, int actionId) {
    std::lock_guard<std::mutex> lock(playerActionsMutex);
    playerActions.emplace_back(playerId, actionId);
}

void AGame::processPlayerActions() {
    for (auto& action : playerActions) {
        int playerId = action.getId();
        int actionId = action.getActionId();

        if (actionId > 0 && actionId < 5) {
            handlePlayerMove(playerId, actionId);
            action.setProcessed(true);
        } else if (actionId == 5) {
            spawnBullet(playerId);
            action.setProcessed(true);
        }
    }
    std::lock_guard<std::mutex> lock(playerActionsMutex);
    deletePlayerAction();
}

void AGame::deletePlayerAction() {
    playerActions.erase(
        std::remove_if(playerActions.begin(), playerActions.end(),
            [](const PlayerAction& action) { return action.getProcessed(); }),
        playerActions.end());
}

const std::vector<PlayerAction>& AGame::getPlayerActions() const {
    return playerActions;
}

std::pair<float, float> AGame::getPlayerPosition(int playerId) const {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::out_of_range("Invalid player ID");
    }

    const auto& positionComponent = it->second.getRegistry().get_components<Position>()[it->second.getEntity()];
    return {positionComponent->x, positionComponent->y};
}

std::pair<float, float> AGame::getBulletPosition(int bulletId) const {
    auto it = bullets.find(bulletId);
    if (it == bullets.end()) {
        throw std::out_of_range("Invalid bullet ID");
    }

    const auto& positionComponent = it->second.getRegistry().get_components<Position>()[it->second.getEntity()];
    return {positionComponent->x, positionComponent->y};
}

void AGame::spawnPlayer(int playerId, float x, float y) {
    players.emplace(playerId, Player(registry, x, y));
    players.at(playerId).updateHp(100.0f);
    players.at(playerId).updateScore(0.0f);

    std::string data = std::to_string(playerId) + ";" + std::to_string(x) + ";" + std::to_string(y) + ";0;0";
    std::string ui_data = std::to_string(playerId) + ";" + std::to_string(players.at(playerId).getHp()) + ";" + std::to_string(players.at(playerId).getScore()) + ";0;0";
    m_server->Broadcast(m_server->createPacket(Network::PacketType::CREATE_PLAYER, data));
    m_server->Broadcast(m_server->createPacket(Network::PacketType::UI_UPDATE, ui_data));
}

void AGame::spawnBullet(int playerId) {
    static int nextBulletId = 0;

    auto it = players.find(playerId);
    if (it != players.end()) {
        auto entity = it->second.getEntity();
        if (it->second.getRegistry().has_component<Position>(entity)) {
            const auto& position = it->second.getRegistry().get_components<Position>()[entity];
            bullets.emplace(nextBulletId, Bullet(registry, nextBulletId, position->x + 50.0f, position->y + 25.0f, 1.0f));
            bulletIdWhoShot[nextBulletId] = playerId;
            std::string data = std::to_string(nextBulletId) + ";" + std::to_string(position->x + 50.0f) + ";" + std::to_string(position->y + 25.0f) + ";5;0";
            m_server->Broadcast(m_server->createPacket(Network::PacketType::CREATE_BULLET, data));
            ++nextBulletId;
        } else {
            std::cerr << "[ERROR] Player does not have a position component." << std::endl;
        }
    } else {
        std::cerr << "[ERROR] Player not found." << std::endl;
    }
}

void AGame::killPlayers(int entityId) {
    for (auto it = players.begin(); it != players.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            it = players.erase(it);
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            m_server->Broadcast(m_server->createPacket(Network::PacketType::DESTROY_PLAYER, data));
            break;
        } else {
            ++it;
        }
    }
}

void AGame::killBullets(int entityId) {
    for (auto it = bullets.begin(); it != bullets.end();) {
        if (it->second.getEntity() == entityId) {
            registry.kill_entity(it->second.getEntity());
            it = bullets.erase(it);
            std::string data = std::to_string(entityId) + ";0;0;0;0";
            m_server->Broadcast(m_server->createPacket(Network::PacketType::DESTROY_BULLET, data));
            break;
        } else {
            ++it;
        }
    }
}

void AGame::killEntity(int entityId) {
    killPlayers(entityId);
    killBullets(entityId);
}
