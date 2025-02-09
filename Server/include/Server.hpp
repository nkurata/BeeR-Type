
#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <thread>
#include <queue>
#include <map>
#include <boost/asio/steady_timer.hpp>

#include "ThreadSafeQueue.hpp"
#include "Packet.hpp"
#include "ClientRegister.hpp"

typedef std::map<uint32_t, ClientRegister> ClientList;

#define MAX_LENGTH 1024

using namespace boost::placeholders; // Used for Boost.Asio asynchronous operations to bind placeholders for callback functions

class Server {
    public:
        Server(boost::asio::io_context& io_context, short port, ThreadSafeQueue<Network::Packet>& packetQueue);
        ~Server();
        using PacketHandler = std::function<void(const std::vector<std::string>&)>;

        void handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred);
        void startReceive();

        void send_to_client(const std::string& message, const boost::asio::ip::udp::endpoint& client_endpoint);
        void Broadcast(const std::string& message);

        Network::ReqConnect reqConnectData(boost::asio::ip::udp::endpoint& client_endpoint);
        Network::DisconnectData disconnectData(boost::asio::ip::udp::endpoint& client_endpoint);
        void sendPong();

        Network::Packet deserializePacket(const std::string& packet_str);
        std::string createPacket(const Network::PacketType& type, const std::string& data);

        const ClientList& getClients() const { return clients_; }
        const udp::endpoint& getRemoteEndpoint() const { return remote_endpoint_; }
        uint32_t createClient(boost::asio::ip::udp::endpoint& client_endpoint);


        void start_send_timer();
        void handle_send_timer(const boost::system::error_code& error);

        void start_print_timer();
        void handle_print_timer(const boost::system::error_code& error);

        ClientList clients_;
        uint32_t _nbClients;
        std::mutex clients_mutex_;
        bool game_running;

        private:
        udp::socket socket_;
        udp::endpoint remote_endpoint_;
        std::array<char, 1024> recv_buffer_;
        ThreadSafeQueue<Network::Packet>& m_packetQueue;
        std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> packet_handlers_;
        std::unordered_map<Network::PacketType, void(*)(const Network::Packet&)> m_handlers;
        std::queue<std::string> send_queue_;
        boost::asio::steady_timer send_timer_;
        std::queue<uint32_t> available_ids_;
        boost::asio::steady_timer print_timer_;
        size_t bytes_recieved_5_seconds;
        int sequence_number = 0;
    };


