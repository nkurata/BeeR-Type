/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** Client
*/

#pragma once

#include "Packet.hpp"

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

namespace RType {
    // Sprite Types
    enum class SpriteType {
        Enemy,
        Boss,
        Player,
        Bullet,
        Background,
        LobbyBackground,
        Start_button,
        Player_Icon
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

        // Game Loop
        int main_loop();
        void processEvents(sf::RenderWindow& window);

        // Input Handling
        void handleKeyPress(sf::Keyboard::Key key, sf::RenderWindow& window);

        // Audio Management
        void adjustVolume(float change);
        void LoadSound();

        // UI & Chat Management
        void updateChat(const std::string& message);
        void drawChat(sf::RenderWindow& window);
        void updateLobbySprites(int numClients);
        void initLobbySprites(sf::RenderWindow& window);

    private:
        // Network Communication
        void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
        void handle_send(const boost::system::error_code& error, std::size_t bytes_transferred);
        void run_receive();
        
        // Sprite Management
        void createSprite();
        void loadTextures();
        void destroySprite();
        
        // Sprite Rendering
        void drawSprites(sf::RenderWindow& window);
        void updateSpritePosition();
        
        // Game State Management
        void resetValues();

        // Member Variables
        sf::RenderWindow window;
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::udp::endpoint server_endpoint_;
        std::array<char, MAX_LENGTH> recv_buffer_;
        std::string received_data;
        std::mutex mutex_;
        std::thread receive_thread_;
        boost::asio::io_context& io_context_;
        std::vector<SpriteElement> sprites_;
        std::unordered_map<SpriteType, sf::Texture> textures_;
        int action;
        int server_id;
        float new_x = 0.0, new_y = 0.0;
        
        // Audio
        sf::SoundBuffer buffer_background_;
        sf::Sound sound_background_;
        sf::SoundBuffer buffer_shoot_;
        sf::Sound sound_shoot_;
        
        // Timers
        boost::asio::steady_timer send_timer_;
        boost::asio::steady_timer receive_timer_;
        
        // Message Queue
        std::queue<std::string> send_queue_;
        
        // Lobby & Chat
        int num_clients_;
        std::vector<sf::Text> player_texts_;
        sf::Font font_;
        std::vector<std::string> chat_messages_;
        const size_t max_chat_messages_ = 10;
        
        int current_ping_ = 0;
    };
}