#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Packet.hpp"
#include "Scene.hpp"
#include "Sprite.hpp"
#include "LobbyScene.hpp"
#include "GameScene.hpp"


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

#define MAX_LENGTH 1024
#define BASE_AUDIO 0

class Client {
    public:
        Client(boost::asio::io_context& io_context, const std::string& host, short server_port, short client_port);
        ~Client();
        int clientLoop();
        // Network Communication
        void send(const std::string& message);
        void start_receive();
        void sendExitPacket();
        void start_send_timer();
        void handle_send_timer(const boost::system::error_code& error);
        void regulate_receive();
        // Packet Handling
        std::string createPacket(Network::PacketType type);
        std::string createMousePacket(Network::PacketType type, int x = 0, int y = 0);
        void parseMessage(std::string packet_data);
        void handleHeartbeatMessage(const std::string& data);
        void setReceiveCallback(std::function<void(const std::string&)> callback);
        //Getters
        int getNumClients();
        int getPing();
        void sendHeartbeatMessage();
        int getAction();
        int getServerId();
        float getNewX();
        float getNewY();
        void switchScene(SceneType scene);

    private:
        void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
        void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred);
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
        int action;
        int server_id;
        float new_x = 0.0, new_y = 0.0;
        std::queue<std::string> send_queue_;
        boost::asio::steady_timer send_timer_;
        boost::asio::steady_timer receive_timer_;
        std::function<void(const std::string&)> receive_callback_;

        int numClients_;
        double ping_ = 0.0;
        int packetSent = 0;
        int packetReceived = 0;
        int packetLost = 0; 
        std::chrono::high_resolution_clock::time_point heartBeatStart_;
        std::chrono::time_point<std::chrono::high_resolution_clock> lastHeartbeatTime_;

        Scene* currentScene = nullptr;

        void start_heartbeat_timer();
        void handle_heartbeat_timer(const boost::system::error_code& error);
};

#endif //CLIENT_HPP