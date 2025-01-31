#include "Client.hpp"
#include "Scene.hpp"
#include "LobbyScene.hpp"
#include <iostream>

using boost::asio::ip::udp;

Client::Client(boost::asio::io_context &io_context, const std::string &host, short server_port, short client_port)
    : socket_(io_context, udp::endpoint(udp::v4(), client_port)), io_context_(io_context), window(sf::VideoMode(1280, 720), "R-Type Client"), send_timer_(io_context), receive_timer_(io_context) // Initialize timers
{
    std::cout << "[DEBUG] Client constructor called" << std::endl;
    udp::resolver resolver(io_context);
    udp::resolver::query query(udp::v4(), host, std::to_string(server_port));
    server_endpoint_ = *resolver.resolve(query).begin();
    std::cout << "Connected to " << host << ":" << server_port << " from client port " << client_port << std::endl;

    regulate_receive(); // Start the regulated receive
    start_send_timer(); // Start the send timer
    receive_thread_ = std::thread(&Client::run_receive, this);
    std::cout << "[DEBUG] Client constructor finished" << std::endl;

    switchScene(SceneType::Lobby); // Start with the Lobby scene
}

Client::~Client()
{
    std::cout << "[DEBUG] Client destructor called" << std::endl;
    io_context_.stop();
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    socket_.close();
    delete currentScene;
    std::cout << "[DEBUG] Client destructor finished" << std::endl;
}

void Client::switchScene(SceneType type) {
    delete currentScene;
    currentScene = nullptr;
    switch (type) {
        case SceneType::Lobby:
            std::cout << "[DEBUG] Switching to Lobby scene" << std::endl;
            currentScene = new LobbyScene(window, *this);
            break;
        case SceneType::Game:
            std::cout << "[DEBUG] Switching to Game scene" << std::endl;
            // currentScene = std::make_unique<GameScene>(window, *this);
            break;
        default:
            // currentScene = std::make_unique<LobbyScene>(window, *this);
            break;
    }
}

// Network Communication
void Client::send(const std::string& message)
{
    socket_.async_send_to(
        boost::asio::buffer(message), server_endpoint_,
        boost::bind(&Client::handle_send, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Client::start_receive()
{
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), server_endpoint_,
        boost::bind(&Client::handle_receive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Client::sendExitPacket()
{
    send(createPacket(Network::PacketType::DISCONNECTED));
}

void Client::start_send_timer() {
    send_timer_.expires_after(std::chrono::milliseconds(1));
    send_timer_.async_wait(boost::bind(&Client::handle_send_timer, this, boost::asio::placeholders::error));
}

void Client::handle_send_timer(const boost::system::error_code& error) {
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

void Client::regulate_receive()
{
    receive_timer_.expires_after(std::chrono::milliseconds(10)); // Set the interval to 10 milliseconds
    receive_timer_.async_wait(boost::bind(&Client::start_receive, this));
}

// private Network Communication
void Client::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!error || error == boost::asio::error::message_size) {
        std::cout << "[DEBUG] Received " << bytes_transferred << " bytes" << std::endl;
        received_data.assign(recv_buffer_.data(), bytes_transferred);
        parseMessage(received_data);
    } else {
        std::cerr << "[DEBUG] Error receiving: " << error.message() << std::endl;
    }
    regulate_receive(); // Regulate the frequency of receive operations
}

void Client::handle_send(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!error) {
        std::cout << "[DEBUG] Sent " << bytes_transferred << " bytes" << std::endl;
    } else {
        std::cerr << "[DEBUG] Error sending: " << error.message() << std::endl;
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

void Client::handleHeartbeatMessage(const std::string& data) {
    std::vector<std::string> elements;
    std::stringstream ss(data);
    std::string segment;
    while (std::getline(ss, segment, ';')) {
        elements.push_back(segment);
    }

    if (elements.size() >= 2) {
        int numClients = std::stoi(elements[0]);
        int ping = std::stoi(elements[1]);
        currentScene->updatePing(ping);
        currentScene->setNumClients(numClients);
        currentScene->addChatLog("Player joined. Total players: " + std::to_string(numClients));
    }
}

int Client::clientLoop()
{
    std::cout << "[DEBUG] Entering main loop" << std::endl;
    send(createPacket(Network::PacketType::REQCONNECT));

    while (this->window.isOpen()) {
        currentScene->processEvents();
        currentScene->update();
        currentScene->render();
        currentScene->renderOverlay(); // Render the ping and chat log overlay

        mutex_.unlock();
    }
    sendExitPacket();
    std::cout << "[DEBUG] Exiting main loop" << std::endl;
    return 0;
}