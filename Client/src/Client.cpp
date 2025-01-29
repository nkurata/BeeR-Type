/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** Client
*/

#include "Client.hpp"

#include <string>

using boost::asio::ip::udp;

RType::Client::Client(boost::asio::io_context& io_context, const std::string& host, short server_port, short client_port)
    : socket_(io_context, udp::endpoint(udp::v4(), client_port)), io_context_(io_context), window(sf::VideoMode(1280, 720), "R-Type Client"), send_timer_(io_context), receive_timer_(io_context) // Initialize timers
{
    std::cout << "[DEBUG] Client constructor called" << std::endl;
    udp::resolver resolver(io_context);
    udp::resolver::query query(udp::v4(), host, std::to_string(server_port));
    server_endpoint_ = *resolver.resolve(query).begin();
    std::cout << "Connected to " << host << ":" << server_port << " from client port " << client_port << std::endl;

    if (!font_.loadFromFile("../assets/font.otf")) {
        std::cerr << "[ERROR] Failed to load font" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    regulate_receive(); // Start the regulated receive
    start_send_timer(); // Start the send timer
    receive_thread_ = std::thread(&Client::run_receive, this);
    std::cout << "[DEBUG] Client constructor finished" << std::endl;
}

RType::Client::~Client()
{
    std::cout << "[DEBUG] Client destructor called" << std::endl;
    io_context_.stop();
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    socket_.close();
    std::cout << "[DEBUG] Client destructor finished" << std::endl;
}

void RType::Client::resetValues()
{
    action = 0;
    server_id = 0;
    new_x = 0.0;
    new_y = 0.0;
}


// Network Communication

void RType::Client::send(const std::string& message)
{
    socket_.async_send_to(
        boost::asio::buffer(message), server_endpoint_,
        boost::bind(&Client::handle_send, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void RType::Client::start_receive()
{
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), server_endpoint_,
        boost::bind(&RType::Client::handle_receive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void RType::Client::sendExitPacket()
{
    send(createPacket(Network::PacketType::DISCONNECTED));
}

void RType::Client::start_send_timer() {
    send_timer_.expires_after(std::chrono::milliseconds(1));
    send_timer_.async_wait(boost::bind(&Client::handle_send_timer, this, boost::asio::placeholders::error));
}

void RType::Client::handle_send_timer(const boost::system::error_code& error) {
    if (!error) {
        if (!send_queue_.empty()) {
            std::string message = send_queue_.front();
            send_queue_.pop();
            send(message);
        }
        start_send_timer();
    } else {
        std::cerr << "[DEBUG] Timer error: " << error.message() << std::endl;
    }

}

void RType::Client::regulate_receive()
{
    receive_timer_.expires_after(std::chrono::milliseconds(10)); // Set the interval to 10 milliseconds
    receive_timer_.async_wait(boost::bind(&Client::start_receive, this));
}

// private Ntework Communication
void RType::Client::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!error || error == boost::asio::error::message_size) {
        std::cout << "[DEBUG] Received " << bytes_transferred << " bytes" << std::endl;
        received_data.assign(recv_buffer_.data(), bytes_transferred);
        parseMessage(received_data);
        if (action == 31) {
            initLobbySprites(this->window);
            action = 0;
        }
    } else {
        std::cerr << "[DEBUG] Error receiving: " << error.message() << std::endl;
    }
    regulate_receive(); // Regulate the frequency of receive operations
}

void RType::Client::handle_send(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!error) {
        std::cout << "[DEBUG] Sent " << bytes_transferred << " bytes" << std::endl;
    } else {
        std::cerr << "[DEBUG] Error sending: " << error.message() << std::endl;
    }
}

void RType::Client::run_receive()
{
    io_context_.run();
}

// Packet Handling
std::string RType::Client::createPacket(Network::PacketType type)
{
    Network::Packet packet;
    packet.type = type;
    std::string packet_str;
    packet_str.push_back(static_cast<uint8_t>(type));
    return packet_str;
}

std::string RType::Client::createMousePacket(Network::PacketType type, int x, int y)
{
    Network::Packet packet;
    packet.type = type;

    std::ostringstream packet_str;
    packet_str << static_cast<uint8_t>(type) << ";" << x << ";" << y;

    return packet_str.str();
}

std::string deserializePacket(const std::string& packet_str)
{
    Network::Packet packet;
    packet.type = static_cast<Network::PacketType>(packet_str[0]);
    return packet_str;
}

void RType::Client::parseMessage(std::string packet_data)
{
    if (packet_data.empty()) {
        std::cerr << "[ERROR] Empty packet data." << std::endl;
        return;
    }
    uint8_t packet_type = static_cast<uint8_t>(packet_data[0]);
    std::string packet_inside = packet_data.substr(2);
    std::cout << "[DEBUG] Received Packet Type: " << static_cast<int>(packet_type) << std::endl;
    std::cout << "[DEBUG] Received Packet Data: " << packet_inside << std::endl;

    if (packet_type == static_cast<uint8_t>(Network::PacketType::HEARTBEAT)) {
        handleHeartbeatMessage(packet_inside);
        return;
    }

    std::vector<std::string> elements;
    std::stringstream ss(packet_inside);
    std::string segment;
    while (std::getline(ss, segment, ';')) {
        elements.push_back(segment);
    }
    if (elements.size() != 3) {
        std::cerr << "[ERROR] Invalid packet format: " << packet_inside << std::endl;
        return;
    }
    try {
        action = static_cast<int>(packet_type);
        server_id = std::stoi(elements[0]);
        new_x = std::stof(elements[1]);
        new_y = std::stof(elements[2]);

        std::cout << "[DEBUG] Action: " << action << std::endl;
        std::cout << "[DEBUG] Server ID: " << server_id << std::endl;
        std::cout << "[DEBUG] New X: " << new_x << std::endl;
        std::cout << "[DEBUG] New Y: " << new_y << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to parse packet data: " << e.what() << std::endl;
    }
}

void RType::Client::handleHeartbeatMessage(const std::string& data) {
    std::vector<std::string> elements;
    std::stringstream ss(data);
    std::string segment;
    while (std::getline(ss, segment, ';')) {
        elements.push_back(segment);
    }

    if (elements.size() >= 2) {
        int numClients = std::stoi(elements[0]);
        int ping = std::stoi(elements[1]);
        std::cout << "[DEBUG] Number of clients: " << numClients << ", Ping: " << ping << "ms" << std::endl;
        updateLobbySprites(numClients);
        current_ping_ = ping;
    }
}

// Input Handling
void RType::Client::handleKeyPress(sf::Keyboard::Key key, sf::RenderWindow& window)
{
    switch (key) {
        case sf::Keyboard::Right:
            std::cout << "[DEBUG] Sending Right: " << std::endl;
            send_queue_.push(createPacket(Network::PacketType::PLAYER_RIGHT));
            break;

        case sf::Keyboard::Left:
            std::cout << "[DEBUG] Sending Left: " << std::endl;
            send_queue_.push(createPacket(Network::PacketType::PLAYER_LEFT));
            break;

        case sf::Keyboard::Up:
            std::cout << "[DEBUG] Sending Up: " << std::endl;
            send_queue_.push(createPacket(Network::PacketType::PLAYER_UP));
            break;

        case sf::Keyboard::Down:
            std::cout << "[DEBUG] Sending Down: " << std::endl;
            send_queue_.push(createPacket(Network::PacketType::PLAYER_DOWN));
            break;

        case sf::Keyboard::Q:
            sendExitPacket();
            window.close();
            break;

        case sf::Keyboard::M:
            std::cout << "[DEBUG] Sending M: " << std::endl;
            send_queue_.push(createPacket(Network::PacketType::OPEN_MENU));
            break;

        case sf::Keyboard::Space:
            std::cout << "[DEBUG] Sending Space: " << std::endl;
            send_queue_.push(createPacket(Network::PacketType::PLAYER_SHOOT));
            break;

        case sf::Keyboard::Escape:
            initLobbySprites(window);
            send_queue_.push(createPacket(Network::PacketType::GAME_END));
            break;

        case sf::Keyboard::Num1:
            adjustVolume(-5);
            break;

        case sf::Keyboard::Num2:
            adjustVolume(5);
            break;

        default:
            break;
    }
}

// UI & Chat Management
void RType::Client::updateChat(const std::string& message) {
    chat_messages_.push_back(message);
    if (chat_messages_.size() > max_chat_messages_) {
        chat_messages_.erase(chat_messages_.begin());
    }
}

void RType::Client::drawChat(sf::RenderWindow& window) {
    float y = window.getSize().y - 20 * chat_messages_.size();
    for (const auto& message : chat_messages_) {
        sf::Text text;
        text.setFont(font_);
        text.setString(message);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);
        text.setPosition(10, y);
        window.draw(text);
        y += 20;
    }

    // Draw the current ping
    sf::Text pingText;
    pingText.setFont(font_);
    pingText.setString("Ping: " + std::to_string(current_ping_) + "ms");
    pingText.setCharacterSize(18);
    pingText.setFillColor(sf::Color::White);
    pingText.setPosition(10, y + 20);
    window.draw(pingText);
}

void RType::Client::updateLobbySprites(int numClients) {
    num_clients_ = numClients;
    initLobbySprites(window);
}

void RType::Client::initLobbySprites(sf::RenderWindow& window)
{
    std::cout << "[DEBUG] Initializing lobby sprites" << std::endl;
    sprites_.clear();
    player_texts_.clear(); // Clear previous texts

    SpriteElement backgroundElement;
    backgroundElement.sprite.setTexture(textures_[SpriteType::LobbyBackground]);
    backgroundElement.sprite.setPosition(0, 0);
    // Scale the background to fit the window size
    float scaleX = static_cast<float>(window.getSize().x) / backgroundElement.sprite.getTexture()->getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / backgroundElement.sprite.getTexture()->getSize().y;
    backgroundElement.sprite.setScale(scaleX, scaleY);
    backgroundElement.id = -100;

    SpriteElement buttonElement;
    buttonElement.sprite.setTexture(textures_[SpriteType::Start_button]);
    buttonElement.sprite.setPosition(window.getSize().x - textures_[SpriteType::Start_button].getSize().x - 20, window.getSize().y - textures_[SpriteType::Start_button].getSize().y - 200); // Position at the bottom right with some padding
    buttonElement.id = -101;

    sprites_.push_back(backgroundElement);
    sprites_.push_back(buttonElement);

    // Add player sprites and texts on the left side of the screen
    for (int i = 0; i < num_clients_; ++i) {
        SpriteElement playerElement;
        playerElement.sprite.setTexture(textures_[SpriteType::Player_Icon]);
        playerElement.sprite.setScale(0.40f, 0.40f);
        playerElement.sprite.setPosition(50, 100 + i * 100);
        playerElement.id = i;
        sprites_.push_back(playerElement);

        sf::Text playerText;
        playerText.setFont(font_);
        playerText.setString("Player " + std::to_string(i + 1));
        playerText.setCharacterSize(24);
        playerText.setFillColor(sf::Color::White);
        // Align text vertically with the middle of the icon
        float textY = playerElement.sprite.getPosition().y + (playerElement.sprite.getGlobalBounds().height / 2) - (playerText.getGlobalBounds().height / 2);
        playerText.setPosition(50 + playerElement.sprite.getGlobalBounds().width + 10, textY); 
        player_texts_.push_back(playerText);
    }
    std::cout << "[DEBUG] Lobby sprites initialized" << std::endl;
}

// Audio
void RType::Client::LoadSound()
{
    if (!buffer_background_.loadFromFile("../assets/sound.wav")) {
        std::cerr << "[ERROR] loading sound file" << std::endl;
    }
    if (!buffer_shoot_.loadFromFile("../assets/shoot.wav")) {
        std::cerr << "[ERROR] loading sound file" << std::endl;
    }
    sound_background_.setBuffer(buffer_background_);
    sound_shoot_.setBuffer(buffer_shoot_);
    sound_shoot_.setVolume(BASE_AUDIO);
    sound_background_.setVolume(BASE_AUDIO);
    sound_background_.setLoop(true);
    sound_background_.play();
}

void RType::Client::adjustVolume(float change)
{
    float newVolume = sound_background_.getVolume() + change;
    if (newVolume < 0) newVolume = 0;
    if (newVolume > 100) newVolume = 100;
    sound_background_.setVolume(newVolume);
}
//Sprite Handling
void RType::Client::drawSprites(sf::RenderWindow& window)
{
    std::cout << "[DEBUG] Drawing sprites" << std::endl;
    for (auto& spriteElement : sprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& text : player_texts_) {
        window.draw(text);
    }
    drawChat(window); // Draw chat messages
    std::cout << "[DEBUG] Finished drawing sprites" << std::endl;
}

void RType::Client::updateSpritePosition()
{
    if (action == 29) {
        for (auto& spriteElement : sprites_) {
            if (server_id == spriteElement.id) {
                spriteElement.sprite.setPosition(new_x, new_y);
                break;
            }
        }
    }
}
//Sprite Management
void RType::Client::createSprite()
{
    SpriteElement spriteElement;
    SpriteType spriteType;

    if (action == 22) { //change by used ID in server to create different types of sprites to be displayed
        spriteType = SpriteType::Enemy;
    } else if (action == 23) {
        spriteType = SpriteType::Boss;
    } else if (action == 24) {
        spriteType = SpriteType::Player;
    } else if (action == 25) {
        spriteType = SpriteType::Bullet;
    } else if (action == 26) {
        spriteType = SpriteType::Background;
    } else {
        return;
    }

    spriteElement.sprite.setTexture(textures_[spriteType]);
    spriteElement.sprite.setPosition(new_x, new_y);
    spriteElement.id = server_id;
    sprites_.push_back(spriteElement);
}

void RType::Client::loadTextures()
{
    textures_[RType::SpriteType::Enemy].loadFromFile("../assets/enemy.png");
    textures_[RType::SpriteType::Boss].loadFromFile("../assets/boss.png");
    textures_[RType::SpriteType::Player].loadFromFile("../assets/player.png");
    textures_[RType::SpriteType::Bullet].loadFromFile("../assets/bullet.png");
    textures_[RType::SpriteType::Background].loadFromFile("../assets/background.png");
    textures_[RType::SpriteType::Start_button].loadFromFile("../assets/play_button.png");
    textures_[RType::SpriteType::LobbyBackground].loadFromFile("../assets/lobby_background.jpg");
    textures_[RType::SpriteType::Player_Icon].loadFromFile("../assets/player_icon.png");
}

void RType::Client::destroySprite()
{
    if (action == 28) {
        for (auto it = sprites_.begin(); it != sprites_.end(); ++it) {
            if (server_id == it->id) {
                sprites_.erase(it);
                break;
            }
        }
    }
    if (action == 3) {
        for (auto it = sprites_.begin(); it != sprites_.end(); ++it) {
            if (it->id == -101) {
                sprites_.erase(it);
            }
        }
    }
}

// Game State Management
void RType::Client::processEvents(sf::RenderWindow& window)
{
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonPressed: {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (!sprites_.empty()) {
                    if (sprites_.back().id == -101) {
                        send_queue_.push(createPacket(Network::PacketType::GAME_START));
                        sprites_.pop_back();
                    } else {
                        send_queue_.push(createMousePacket(Network::PacketType::PLAYER_SHOOT, mousePos.x, mousePos.y));
                        sound_shoot_.play();
                    }
                }
                break;
            }
            case sf::Event::KeyPressed:
                handleKeyPress(event.key.code, window);
                break;

            default:
                break;
        }
    }
}

int RType::Client::main_loop()
{
    std::cout << "[DEBUG] Entering main loop" << std::endl;
    loadTextures();
    send(createPacket(Network::PacketType::REQCONNECT));
    LoadSound();

    while (this->window.isOpen()) {
        processEvents(this->window);
        createSprite();
        destroySprite();
        updateSpritePosition();
        resetValues();
        mutex_.unlock();

        this->window.clear();
        drawSprites(window);
        this->window.display();
    }
    sendExitPacket();
    std::cout << "[DEBUG] Exiting main loop" << std::endl;
    return 0;
}