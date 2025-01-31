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


#define MAX_LENGTH 1024
#define BASE_AUDIO 0

// Sprite Types
enum class SpriteType {
    Enemy,
    Boss,
    Player,
    Bullet,
    Background,
    LobbyBackground,
    StartButton,
    PlayerIcon
};

class SpriteElement {
public:
    sf::Sprite sprite;
    int id;
};

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

        void getNumClients();

    private:
        void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
        void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred);
        void run_receive();

        void switchScene(SceneType scene);

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

        Scene* currentScene;
};

#endif //CLIENT_HPP