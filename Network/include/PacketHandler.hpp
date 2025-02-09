#ifndef PACKET_HANDLER_HPP
#define PACKET_HANDLER_HPP

#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include "ThreadSafeQueue.hpp"
#include "Packet.hpp"
#include "PacketType.hpp"
#include "Game.hpp"
#include "Server.hpp"

namespace Network {
    class PacketHandler {
    public:
        PacketHandler(ThreadSafeQueue<Network::Packet>& queue, Game& game, Server& server);
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
        void handlePlayerRight(const Network::Packet &packet);
        void handlePlayerLeft(const Network::Packet &packet);
        void handlePlayerUp(const Network::Packet &packet);
        void handlePlayerDown(const Network::Packet &packet);
        void handlePlayerAction(const Network::Packet &packet, int action);
        void handlePing(const Network::Packet &packet);
        void handlePlayerShoot(const Network::Packet &packet);
        void handleGameEnd(const Network::Packet &packet);

    private:
        ThreadSafeQueue<Network::Packet> &m_queue;
        Game& m_game;
        std::thread m_thread;
        Server& server_;
        std::atomic<bool> m_running{false};
        std::mutex m_mutex;

        std::unordered_map<Network::PacketType, std::function<void(const Network::Packet&)>> m_handlers;
    };
}

#endif // PACKET_HANDLER_HPP