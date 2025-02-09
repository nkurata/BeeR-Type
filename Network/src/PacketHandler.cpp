#include "PacketHandler.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <functional>

using namespace Network;

// Constructor
PacketHandler::PacketHandler(ThreadSafeQueue<Network::Packet>& queue, Game& game, Server& server) : m_queue(queue), m_game(game), server_(server)
{
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
    m_handlers[Network::PacketType::GAME_START] = std::bind(&PacketHandler::handleGameStart, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_UP] = std::bind(&PacketHandler::handlePlayerUp, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_DOWN] = std::bind(&PacketHandler::handlePlayerDown, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_LEFT] = std::bind(&PacketHandler::handlePlayerLeft, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_RIGHT] = std::bind(&PacketHandler::handlePlayerRight, this, std::placeholders::_1);
    m_handlers[Network::PacketType::PLAYER_SHOOT] = std::bind(&PacketHandler::handlePlayerShoot, this, std::placeholders::_1);
    m_handlers[Network::PacketType::GAME_END] = std::bind(&PacketHandler::handleGameEnd, this, std::placeholders::_1);
    m_handlers[Network::PacketType::HEARTBEAT] = std::bind(&PacketHandler::handlePing, this, std::placeholders::_1);
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
    std::cout << "[PacketHandler] Handled NONE packet." << std::endl;
}

void PacketHandler::reqConnect(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto endpoint = server_.getRemoteEndpoint();
    server_.reqConnectData(endpoint);
}

void PacketHandler::handleDisconnected(const Network::Packet &packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto endpoint = server_.getRemoteEndpoint();
    server_.disconnectData(endpoint);
}

void PacketHandler::handleGameStart(const Network::Packet &packet)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (server_.game_running) {
            std::cout << "[PacketHandler] Game already running, ignoring GAME_START packet." << std::endl;
            return;
        }
        std::cout << "[PacketHandler] Handled GAME_START packet." << std::endl;
        server_.game_running = true;
        const auto& clients = server_.getClients();
        int numPlayers = clients.size();
        server_.Broadcast(server_.createPacket(Network::PacketType::GAME_START, ""));

        std::thread gameThread([this, numPlayers] {
            m_game.run(numPlayers);
        });
        gameThread.detach();
    }
}

void PacketHandler::handleGameEnd(const Network::Packet &packet)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!server_.game_running) {
            std::cout << "[PacketHandler] Game not running, ignoring GAME_END packet." << std::endl;
            return;
        }
        std::cout << "[PacketHandler] Handled GAME_END packet." << std::endl;
        server_.game_running = false;
        m_game.stop();
    }
}

void PacketHandler::handlePlayerShoot(const Network::Packet &packet)
{
    const auto& clients = server_.getClients();
    const udp::endpoint& clientEndpoint = server_.getRemoteEndpoint();

    size_t playerId = -1;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(server_.clients_mutex_);

        for (const auto& [id, client] : clients) {
            if (client.getEndpoint() == clientEndpoint) {
                playerId = id;
                found = true;
                break;
            }
        }
    } if (found) {
        m_game.spawnBullet(playerId);
    } else {
        std::cerr << "[PacketHandler] Client endpoint not found in client list." << std::endl;
    }
}

void PacketHandler::handlePlayerRight(const Network::Packet &packet)
{
    handlePlayerAction(packet, 2);
}

void PacketHandler::handlePlayerLeft(const Network::Packet &packet)
{
    handlePlayerAction(packet, 1);
}

void PacketHandler::handlePlayerUp(const Network::Packet &packet)
{
    handlePlayerAction(packet, 3);
}

void PacketHandler::handlePlayerDown(const Network::Packet &packet)
{
    handlePlayerAction(packet, 4);
}

void PacketHandler::handlePing(const Network::Packet &packet)
{
    server_.sendPong();
}

void PacketHandler::handlePlayerAction(const Network::Packet &packet, int action)
{
    if (!server_.game_running) {
        std::cout << "[PacketHandler] game is not running, cannot move." << std::endl;
        return;
    }
    const auto& clients = server_.getClients();
    const udp::endpoint& clientEndpoint = server_.getRemoteEndpoint();

    size_t playerId = -1;
    bool found = false;
    {
        std::lock_guard<std::mutex> lock(server_.clients_mutex_);

        for (const auto& [id, client] : clients) {
            if (client.getEndpoint() == clientEndpoint) {
                playerId = id;
                found = true;
                break;
            }
        }
    }
    if (found) {
        m_game.addPlayerAction(playerId, action);
    } else {
        std::cerr << "[PacketHandler] Client endpoint not found in client list." << std::endl;
    }
}
