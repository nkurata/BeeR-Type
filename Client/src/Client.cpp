#include "Client.hpp"
#include "Scene.hpp"
#include "LobbyScene.hpp"
#include "GameScene.hpp"
#include <iostream>

using boost::asio::ip::udp;

Client::Client(boost::asio::io_context &io_context, const std::string &host, short server_port, short client_port)
    : socket_(io_context, udp::endpoint(udp::v4(), client_port)),
    io_context_(io_context), window(sf::VideoMode(1280, 720), "R-Type Client"),
    send_timer_(io_context),
    receive_timer_(io_context),
    currentScene(nullptr),
    lastHeartbeatTime_(std::chrono::high_resolution_clock::now())
{
    // std::cout << "[DEBUG] Client constructor called" << std::endl;
    udp::resolver resolver(io_context);
    udp::resolver::query query(udp::v4(), host, std::to_string(server_port));
    server_endpoint_ = *resolver.resolve(query).begin();
    std::cout << "Connected to " << host << ":" << server_port << " from client port " << client_port << std::endl;

    startReceive(); // Start the regulated receive
    startSendTimer(); // Start the send timer
    receive_thread_ = std::thread(&Client::run_receive, this);
    // std::cout << "[DEBUG] Client constructor finished" << std::endl;

    switchScene(SceneType::Lobby); // Start with the Lobby scene
}

Client::~Client()
{
    // std::cout << "[DEBUG] Client destructor called" << std::endl;
    io_context_.stop();
    socket_.close();
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    currentScene.reset();
    // std::cout << "[DEBUG] Client destructor finished" << std::endl;
}

void Client::resetValues()
{
    action = 0;
    server_id = 0;
    new_x = 0.0;
    new_y = 0.0;
}

void Client::switchScene(SceneType type) {
    currentScene.reset(); // Automatically deletes the current scene

    switch (type) {
        case SceneType::Lobby:
            currentScene = std::make_unique<LobbyScene>(window, *this);
            send_queue_.push(createPacket(Network::PacketType::GAME_END));
            break;
        case SceneType::Game:
            std::cout << "[DEBUG] Switching to GameScene" << std::endl;
            send_queue_.push(createPacket(Network::PacketType::GAME_START));
            currentScene = std::make_unique<GameScene>(window, *this);
            std::cout << "[DEBUG] GameScene created" << std::endl;
            break;
        default:
            std::cerr << "[ERROR] Unknown scene type" << std::endl;
            break;
    }
}

// Getters
int Client::getPing() {
    return ping_;
}

int Client::getNumClients() {
    return numClients_;
}

// Network Communication
void Client::send(const std::string& message)
{
    packetSent++;
    socket_.async_send_to(
        boost::asio::buffer(message), server_endpoint_,
        boost::bind(&Client::handleSend, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Client::startReceive()
{
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), server_endpoint_,
        boost::bind(&Client::handleReceive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Client::sendExitPacket()
{
    send(createPacket(Network::PacketType::DISCONNECTED));
}

void Client::startSendTimer() {
    send_timer_.expires_after(std::chrono::milliseconds(1));
    send_timer_.async_wait(boost::bind(&Client::handleSendTimer, this, boost::asio::placeholders::error));
}

void Client::handleSendTimer(const boost::system::error_code& error) {
    if (!error) {
        if (!send_queue_.empty()) {
            std::string message = send_queue_.front();
            send_queue_.pop();
            send(message);
        }
        startSendTimer();
    } else {
        std::cerr << "[ERROR] Timer error: " << error.message() << std::endl;
    }
}

void Client::regulate_receive()
{
    receive_timer_.expires_after(std::chrono::milliseconds(10)); // Set the interval to 10 milliseconds
    receive_timer_.async_wait(boost::bind(&Client::startReceive, this));
}

// private Network Communication
void Client::handleReceive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (!error || error == boost::asio::error::message_size) {
        mutex_.lock();
        packetReceived++;
        received_data.assign(recv_buffer_.data(), bytes_transferred);
        parseMessage(received_data);
        startReceive();
    } else {
        std::cerr << "[ERROR] Error receiving: " << error.message() << std::endl;
        startReceive();
    }
}

void Client::handleSend(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (!error) {
        std::cout << "[DEBUG] Sent " << bytes_transferred << " bytes" << std::endl;
    } else {
        packetLost++;
        std::cerr << "[ERROR] Error sending: " << error.message() << std::endl;
    }
}

void Client::run_receive()
{
    io_context_.run();
}

// Packet Handling
std::string Client::createPacket(Network::PacketType type)
{
    Network::Packet packet;
    packet.type = type;
    std::string packet_str;
    packet_str.push_back(static_cast<uint8_t>(type));
    return packet_str;
}

std::string Client::createMousePacket(Network::PacketType type, int x, int y)
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

void Client::parseMessage(std::string packet_data)
{
    if (packet_data.empty()) {
        std::cerr << "[ERROR] Empty packet data." << std::endl;
        return;
    }
    uint8_t packet_type = static_cast<uint8_t>(packet_data[0]);
    std::string packet_inside = packet_data.substr(2);

    if (packet_type == static_cast<uint8_t>(Network::PacketType::HEARTBEAT)) {
        handleHeartbeatMessage(packet_inside);
        return;
    } if (packet_type == static_cast<uint8_t>(Network::PacketType::GAME_START)) {
        switchScene(SceneType::Game);
        return;
    } if (packet_type == static_cast<uint8_t>(Network::PacketType::GAME_END)) {
        switchScene(SceneType::Lobby);
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

void Client::sendHeartbeatMessage()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = now - lastHeartbeatTime_;
    
    if (elapsed.count() > 100) {
        auto start = std::chrono::high_resolution_clock::now();
        send_queue_.push(createPacket(Network::PacketType::HEARTBEAT));
        heartBeatStart_ = start;
        lastHeartbeatTime_ = now;
    }
}

void Client::handleHeartbeatMessage(const std::string& data) {
    std::vector<std::string> elements;
    std::stringstream ss(data);
    std::string segment;
    while (std::getline(ss, segment, ';')) {
        elements.push_back(segment);
    }

    if (elements.size() > 0) {
        int newNumClients = std::stoi(elements[0]);
        if (newNumClients > numClients_) {
            currentScene->addChatLog("Player connected. Total players: " + std::to_string(newNumClients));
        } else if (newNumClients < numClients_) {
            currentScene->addChatLog("Player disconnected. Total players: " + std::to_string(newNumClients));
        }
        numClients_ = newNumClients;
    }

    // Calculate the ping as the round-trip time
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = now - heartBeatStart_;
    ping_ = elapsed.count();

    // Print the content of the heartbeat message
    // std::cout << "[DEBUG] Heartbeat message content: " << data << std::endl;
    // std::cout << "[DEBUG] Calculated ping: " << ping_ << " ms" << std::endl;
}

int Client::clientLoop()
{
    send_queue_.push(createPacket(Network::PacketType::REQCONNECT));

    while (this->window.isOpen()) {
        std::cout << "[DEBUG] Client loop running" << std::endl;
        currentScene->processEvents();
        std::cout << "[DEBUG] Events processed" << std::endl;
        currentScene->update();
        std::cout << "[DEBUG] Scene updated" << std::endl;
        currentScene->render();
        std::cout << "[DEBUG] Scene rendered" << std::endl;
        resetValues();
        std::cout << "[DEBUG] Values reset" << std::endl;
        sendHeartbeatMessage();
        std::cout << "[DEBUG] Heartbeat message sent" << std::endl;
        mutex_.unlock();
        std::cout << "[DEBUG] Mutex unlocked" << std::endl;
    }
    sendExitPacket();
    return 0;
}
