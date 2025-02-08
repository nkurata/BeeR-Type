/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** PacketHandler
*/

#include "PacketHandler.hpp"
#include "Server.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <functional>
#include <dlfcn.h>

using namespace Network;

// Constructor
PacketHandler::PacketHandler(ThreadSafeQueue<Network::Packet>& queue, Server& server, Game& game) : m_queue(queue), m_server(server), m_game(game) {
    initializeHandlers();
}

PacketHandler::~PacketHandler() {
    stop();
}

void PacketHandler::start() {
    m_running = true;
    m_thread = std::thread(&PacketHandler::processPackets, this);
}

void PacketHandler::stop() {
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

void PacketHandler::processPackets() {
    while (m_running) {
        Network::Packet packet = m_queue.pop();
        handlePacket(packet);
    }
}

void PacketHandler::initializeHandlers() {
    m_handlers[Network::PacketType::NONE] = std::bind(&PacketHandler::handleNone, this, std::placeholders::_1);
    m_handlers[Network::PacketType::REQCONNECT] = std::bind(&PacketHandler::reqConnect, this, std::placeholders::_1);
    m_handlers[Network::PacketType::DISCONNECTED] = std::bind(&PacketHandler::handleDisconnected, this, std::placeholders::_1);
    m_handlers[Network::PacketType::CONNECTED] = std::bind(&PacketHandler::handleConnected, this, std::placeholders::_1);
    m_handlers[Network::PacketType::GAME_START] = std::bind(&PacketHandler::handleGameStart, this, std::placeholders::_1);
    m_handlers[Network::PacketType::GAME_END] = std::bind(&PacketHandler::handleGameEnd, this, std::placeholders::_1);

    m_handlers[Network::PacketType::PLAYER_UP] = std::bind(&PacketHandler::handlePlayerUp, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_DOWN] = std::bind(&PacketHandler::handlePlayerDown, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_LEFT] = std::bind(&PacketHandler::handlePlayerLeft, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_RIGHT] = std::bind(&PacketHandler::handlePlayerRight, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_SHOOT] = std::bind(&PacketHandler::handlePlayerShoot, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_BLAST] = std::bind(&PacketHandler::handlePlayerBlast, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_STOP] = std::bind(&PacketHandler::handlePlayerStop, this, std::placeholders::_1);

    m_handlers[Network::PacketType::HEARTBEAT] = std::bind(&PacketHandler::handleHeartbeat, this, std::placeholders::_1);
}

void PacketHandler::handlePacket(const Network::Packet &packet) {
    auto it = m_handlers.find(packet.type);
    if (it != m_handlers.end()) {
        it->second(packet);
    } else {
        std::cout << "[PacketHandler] Received unknown packet type." << std::endl;
    }
}

void PacketHandler::handleNone(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[PacketHandler] Handled NONE packet." << std::endl;
}

void PacketHandler::reqConnect(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[PacketHandler] Handled CONNECTED packet." << std::endl;
    auto endpoint = m_server.getRemoteEndpoint();
    m_server.reqConnectData(endpoint);
}

void PacketHandler::handleDisconnected(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[PacketHandler] Handled DISCONNECTED packet." << std::endl;
    auto endpoint = m_server.getRemoteEndpoint();
    m_server.disconnectData(endpoint);
}

void PacketHandler::handleConnected(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[PacketHandler] Handled CONNECTED packet." << std::endl;
}	

typedef IGame *createGame_t(Server &server);

IGame *getGameInstance(const std::string &path, Server &server)
{
    void *handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw std::runtime_error(dlerror());
    }
    auto *createGame = (createGame_t *)dlsym(handle, "createGame");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        throw std::runtime_error(dlsym_error);
    } if (!createGame) {
        throw std::runtime_error(dlerror());
    } if (handle) {
        dlclose(handle);
    }
    return createGame(server);
}

void PacketHandler::handleGameStart(const Network::Packet &packet)
{
    int numPlayers;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout << "[PacketHandler] Handled GAME_START packet." << std::endl;
        m_server.m_running = true;
        const auto& clients = m_server.getClients();
        numPlayers = clients.size();
        m_server.Broadcast(m_server.createPacket(Network::PacketType::GAME_START, ""));
    }
    std::thread gameThread([this, numPlayers] {
        try {
            m_game.run(numPlayers);
        } catch (const std::exception &e) {
            std::cerr << "[PacketHandler] Failed to start game: " << e.what() << std::endl;
        }
    });
    gameThread.detach();
}

void PacketHandler::handlePlayerShoot(const Network::Packet &packet)
{
    // std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[PacketHandler] Handled PLAYER_SHOOT packet." << std::endl;
    const auto& clients = m_server.getClients();
    const udp::endpoint& clientEndpoint = m_server.getRemoteEndpoint();

    size_t playerId = -1;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(m_server.clients_mutex_);

        for (const auto& [id, client] : clients) {
            if (client.getEndpoint() == clientEndpoint) {
                playerId = id;
                found = true;
                break;
            }
        }
    } if (found) {
        // m_game.spawnBullet(playerId);
    } else {
        std::cerr << "[PacketHandler] Client endpoint not found in client list." << std::endl;
    }
}

void PacketHandler::handlePlayerBlast(const Network::Packet & packet) {
    std::cout << "[PacketHandler] Handled PLAYER_BLAST packet." << std::endl;  
}

void PacketHandler::handleGameEnd(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[PacketHandler] Handeled GAME_END packet." << std::endl;
}

void PacketHandler::handlePlayerUp(const Network::Packet &packet)
{
    std::cout << "[PacketHandler] Handled PLAYER_UP packet." << std::endl;
    handlePlayerAction(packet, Network::PacketType::PLAYER_UP);
}

void PacketHandler::handlePlayerDown(const Network::Packet &packet)
{
    std::cout << "[PacketHandler] Handeled PLAYER_DOWN packet." << std::endl;
    handlePlayerAction(packet, Network::PacketType::PLAYER_DOWN);
}

void PacketHandler::handlePlayerLeft(const Network::Packet &packet)
{
    std::cout << "[PacketHandler] Handeled PLAYER_LEFT packet." << std::endl;
    handlePlayerAction(packet, Network::PacketType::PLAYER_LEFT);
}

void PacketHandler::handlePlayerRight(const Network::Packet &packet)
{
    std::cout << "[PacketHandler] Handeled PLAYER_RIGHT packet." << std::endl;
    handlePlayerAction(packet, Network::PacketType::PLAYER_RIGHT);
}

void PacketHandler::handlePlayerStop(const Network::Packet &packet)
{
    std::cout << "[PacketHandler] Handeled PLAYER_RIGHT packet." << std::endl;
    handlePlayerAction(packet, Network::PacketType::PLAYER_STOP);
}

void PacketHandler::handleHeartbeat(const Network::Packet &packet) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_server.sendHeartbeatMessage();
}

void PacketHandler::handlePlayerAction(const Network::Packet &packet, Network::PacketType action)
{
    {
        std::lock_guard<std::mutex> lock(m_server.clients_mutex_);
        if (!m_server.m_running) {
            std::cout << "[PacketHandler] game is not running, cannot move." << std::endl;
            return;
        }
    }
    std::cout << "[PacketHandler] Handling player action: " << static_cast<int>(action) << std::endl;
    const auto& clients = m_server.getClients();
    const udp::endpoint& clientEndpoint = m_server.getRemoteEndpoint();

    size_t playerId = -1;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(m_server.clients_mutex_);

        for (const auto& [id, client] : clients) {
            if (client.getEndpoint() == clientEndpoint) {
                playerId = id;
                found = true;
                break;
            }
        }
    }
    if (found) {
        m_game.addPlayerAction(playerId, static_cast<int>(action));
    } else {
        std::cerr << "[PacketHandler] Client endpoint not found in client list." << std::endl;
    }
}
