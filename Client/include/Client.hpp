#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Packet.hpp"
#include "Scene.hpp"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/array.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <array>
#include <thread>
#include <mutex>
#include <csignal>
#include <unordered_map>
#include <boost/asio/steady_timer.hpp>
#include <queue>
#include <chrono>
#include <vector>


#define MAX_LENGTH 1024
#define BASE_AUDIO 0

// Sprite Type

class Client {
    public:
        Client(boost::asio::io_context& io_context, const std::string& host, short server_port, short client_port);
        ~Client();
        int clientLoop();
        // Network Communication
        void send(const std::string& message);
        void startReceive();
        void sendExitPacket();
        void startSendTimer();
        void handleSendTimer(const boost::system::error_code& error);
        void regulate_receive();
        void startResetTimer();
        void handleResetTimer(const boost::system::error_code& error);
        // Packet Handling
        std::string createPacket(Network::PacketType type);
        std::string createMousePacket(Network::PacketType type, int x = 0, int y = 0);
        void parseMessage(std::string packet_data);
        void handleHeartbeatMessage(std::vector<std::string> elements);
        void resetValues();
        //Getters
        int getNumClients();
        int getPing();
        void sendHeartbeatMessage();
        void switchScene(SceneType scene);

        int action;
        int server_id;
        float new_x = 0.0, new_y = 0.0;
        int packetLost = 0;
        std::queue<std::string> send_queue_;

    private:
        void handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred);
        void handleSend(const boost::system::error_code& error, std::size_t bytes_transferred);
        void run_receive();

        // variables
        sf::RenderWindow window;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint server_endpoint_;
        std::array<char, MAX_LENGTH> recv_buffer_;
        std::string received_data;
        std::mutex mutex_;
        std::thread receive_thread_;
        boost::asio::io_context& io_context_;
        boost::asio::steady_timer send_timer_;
        boost::asio::steady_timer receive_timer_;
        boost::asio::steady_timer reset_timer_;
        std::function<void(const std::string&)> receive_callback_;

        int numClients_;
        double ping_ = 0.0;
        int packetSent = 0;
        int packetReceived = 0;
        int lastPacketnb = 0;
        std::chrono::high_resolution_clock::time_point heartBeatStart_;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastHeartbeatTime_;

        std::unique_ptr<Scene> currentScene;
};

#endif //CLIENT_HPP