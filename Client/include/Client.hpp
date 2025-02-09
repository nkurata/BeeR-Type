#pragma once

#include "Packet.hpp"
#include "LobbyScene.hpp"
#include "GameScene.hpp"
#include "Sprite.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <SFML/Graphics.hpp>
#include <boost/array.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <boost/asio.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <iostream>
#include <csignal>
#include <iomanip>
#include <vector>
#include <thread>
#include <array>
#include <mutex>
#include <queue>

#define MAX_LENGTH 1024


    class Client {
    public:
        Client(boost::asio::io_context& io_context, const std::string& host, short server_port, short client_port);
        ~Client();
        int main_loop();

        void startReceive();
        void handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred);
        void handleSend(const boost::system::error_code& error, std::size_t bytes_transferred);
        void send(const std::string& message);


        void sendExitPacket() { send(createPacket(Network::PacketType::DISCONNECTED)); }

        void handleHeartBeat();
        void sendHeartBeat();

        void runReceive();
        void parseMessage();
        std::string createPacket(Network::PacketType type);
        std::vector<std::string> deserialisePacket(std::string packet_inside);

        void startSendTimer();
        void handleSendTimer(const boost::system::error_code& error);

        double resetPacketLoss();

        void stop();

    private:
        PacketData packet_data_;
        LagMeter lag_meter_;

        //server
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint server_endpoint_;
        std::array<char, MAX_LENGTH> recv_buffer_;
        std::string received_data_;
        std::mutex mutex_;
        std::thread receive_thread_;
        boost::asio::io_context& io_context_;
        boost::asio::steady_timer send_timer_;
        std::queue<std::string> send_queue_;

        //client sfml
        sf::RenderWindow window_;

        //ping variables
        int last_packetnb_ = 0;
        int packets_sent_ = 0;
        int packets_received_ = 0;
        std::chrono::high_resolution_clock::time_point heartbeat_start_time_;
        std::chrono::time_point<std::chrono::high_resolution_clock> last_heartbeat_time_;
        sf::Clock game_clock_;

        boost::asio::steady_timer heartbeat_timer_;
        bool awaiting_ = false;
        std::chrono::high_resolution_clock::time_point last_response_time_;

        void schedulePing();
        void checkTimeout();

    };
