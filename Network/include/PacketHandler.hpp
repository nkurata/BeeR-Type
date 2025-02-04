/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** PacketHandler
*/

#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include "ThreadSafeQueue.hpp"
#include "Packet.hpp"
#include "PacketType.hpp"
#include "Server.hpp"
#include "Game.hpp"

namespace Network {
    class PacketHandler {
    public:
        PacketHandler(ThreadSafeQueue<Network::Packet>& queue, Server& server, Game& game);
        ~PacketHandler();

        void start();
        void stop();
        void processPackets();

        void handlePacket(const Network::Packet &packet);
        void initializeHandlers();

        // handler functions for each packet type
        void handleNone(const Network::Packet &packet);
        void reqConnect(const Network::Packet &packet);
        void handleDisconnected(const Network::Packet &packet);
        void handleGameStart(const Network::Packet &packet);
        void handlePlayerDead(const Network::Packet &packet);
        void handlePlayerJoin(const Network::Packet &packet);
        void handlePlayerShoot(const Network::Packet &packet);
        void handlePlayerHit(const Network::Packet &packet);
        void handlePlayerScore(const Network::Packet &packet);
        void handleEnemySpawned(const Network::Packet &packet);
        void handleEnemyDead(const Network::Packet &packet);
        void handleEnemyMoved(const Network::Packet &packet);
        void handleEnemyShoot(const Network::Packet &packet);
        void handleEnemyLifeUpdate(const Network::Packet &packet);
        void handleMapUpdate(const Network::Packet &packet);
        void handleGameEnd(const Network::Packet &packet);
        void handlePlayerRightStart(const Network::Packet &packet);
        void handlePlayerLeftStart(const Network::Packet &packet);
        void handlePlayerUpStart(const Network::Packet &packet);
        void handlePlayerDownStart(const Network::Packet &packet);
        void handlePlayerRightStop(const Network::Packet &packet);
        void handlePlayerLeftStop(const Network::Packet &packet);
        void handlePlayerUpStop(const Network::Packet &packet);
        void handlePlayerDownStop(const Network::Packet &packet);
        void handleOpenMenu(const Network::Packet &packet);
        void handlePlayerAction(const Network::Packet &packet, int action);
        void handleHeartbeat(const Network::Packet &packet);
        void handlePOCStart(const Network::Packet &packet);

    private:
        ThreadSafeQueue<Network::Packet> &m_queue;
        Server& m_server;
        std::thread m_thread;
        std::atomic<bool> m_running{false};
        std::mutex m_mutex;
        Game& m_game;

        std::unordered_map<Network::PacketType, std::function<void(const Network::Packet&)>> m_handlers;
    };
}

#endif // PACKET_HANDLER_HPP
