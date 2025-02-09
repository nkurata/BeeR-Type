/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** server
*/

#include "Server.hpp"

using boost::asio::ip::udp;

/**
 * @brief Constructs a new Server object.
 *
 * This constructor initializes the UDP socket and starts the asynchronous
 * receive operation to handle incoming data.
 *
 * @param io_context The io_context object used for asynchronous operations.
 * @param port The port number on which the server will listen for incoming UDP packets.
 */
Server::Server(boost::asio::io_context& io_context, short port, ThreadSafeQueue<Network::Packet>& packetQueue)
: socket_(io_context, udp::endpoint(udp::v4(), port)), m_packetQueue(packetQueue), _nbClients(0), m_running(false), send_timer_(io_context), receive_timer_(io_context) // Initialize timers
{
    regulate_receive();
    startSendTimer();
}

Server::~Server()
{
    socket_.close();
}

// void Server::setGameState(GameState* game) {
//     m_game = game;
// }
//SEND MESSAGES

void Server::send_to_client(const std::string& message, const udp::endpoint& client_endpoint)
{
    socket_.async_send_to(
        boost::asio::buffer(message), client_endpoint,
        [](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                // std::cout << "[DEBUG] Message sent to client." << std::endl;
            } else {
                std::cerr << "[ERROR] Error sending to client: " << error.message() << std::endl;
            }
        });
}

void Server::Broadcast(const std::string& message)
{
    {
        send_queue_.push(message); // Add to queue
    }
}

/**
 * @brief Starts an asynchronous receive operation.
 *
 * This function initiates an asynchronous receive operation to receive data
 * from a remote endpoint. When data is received, the provided handler function
 * is called to process the received data.
 */
void Server::startReceive()
{
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&Server::handleReceive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

/**
 * @brief Handles the completion of an asynchronous receive operation.
 *
 * This function is called when data is received from a remote endpoint. It processes
 * the received data, optionally sends a response back to the client, and restarts
 * the asynchronous receive operation to handle the next incoming message.
 *
 * @param error The error code indicating the result of the receive operation.
 * @param bytes_transferred The number of bytes received.
 */

void Server::handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred)
{
    if (!error || error == boost::asio::error::message_size) {
        std::string received_data(recv_buffer_.data(), bytes_transferred);
        // std::cout << "[DEBUG] Received: " << static_cast<int>(received_data[0]) << std::endl;

        Network::Packet packet;
        packet.type = deserializePacket(received_data).type;
        m_packetQueue.push(packet);
    } else {
        std::cerr << "[ERROR] Error receiving: " << error.message() << std::endl;
    }
    regulate_receive(); // Regulate the frequency of receive operations
}

Network::Packet Server::deserializePacket(const std::string& packet_str)
{
    Network::Packet packet;
    packet.type = static_cast<Network::PacketType>(packet_str[0]);
    return packet;
}

std::string Server::createPacket(const Network::PacketType& type, const std::string& data) {
    std::string packet_str;
    std::string packet_data = data.empty() ? "-1;-1;-1" : data;
    std::cout << "[DEBUG] Creating packet with type: " << static_cast<int>(type) << " and data: " << packet_data << std::endl;

    packet_str.push_back(static_cast<uint8_t>(type));
    packet_str.push_back(static_cast<uint8_t>(';'));
    for (char c : packet_data) {
        packet_str.push_back(static_cast<uint8_t>(c));
    }
    packet_str.push_back(static_cast<uint8_t>(';'));
    packet_str += std::to_string(++packetNb); // Add packet number
    return packet_str;
}

//COMMANDS

uint32_t Server::createClient(boost::asio::ip::udp::endpoint& client_endpoint)
{
    uint32_t nb;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);

        for (const auto& client : clients_) {
            if (client.second.getEndpoint() == client_endpoint)
                return client.first;
        }
        if (!available_ids_.empty()) {
            nb = available_ids_.front();
            available_ids_.pop();
        } else
            nb = this->_nbClients++;
        ClientRegister newClient(nb, client_endpoint);
        clients_.insert(std::make_pair(nb, newClient));
    }
    return nb;
}

Network::ReqConnect Server::reqConnectData(boost::asio::ip::udp::endpoint& client_endpoint)
{
    Network::ReqConnect data;
    size_t idClient;
    idClient = createClient(client_endpoint);
    data.id = idClient;
    {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    send_to_client(createPacket(Network::PacketType::CONNECTED, ""), client_endpoint);
    return data;
    }
}

Network::DisconnectData Server::disconnectData(boost::asio::ip::udp::endpoint& client_endpoint)
{
    Network::DisconnectData data;
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);

        for (auto it = clients_.begin(); it != clients_.end(); ++it) {
            if (it->second.getEndpoint() == client_endpoint) {
                data.id = it->second.getId();
                // std::cout << "[DEBUG] Client " << data.id << " disconnected." << std::endl;

                available_ids_.push(data.id);

                clients_.erase(it);
                return data;
            }
        }
    }
    data.id = -1;
    std::cerr << "[ERROR] Client not found." << std::endl;
    send_to_client(createPacket(Network::PacketType::NONE, ""), client_endpoint);
    return data;
}

bool Server::hasPositionChanged(int id, float x, float y, std::unordered_map<int, std::pair<float, float>>& lastKnownPositions) {
    auto it = lastKnownPositions.find(id);
    if (it == lastKnownPositions.end() || it->second != std::make_pair(x, y)) {
        lastKnownPositions[id] = {x, y};
        return true;
    }
    return false;
}


void Server::startSendTimer() {
    send_timer_.expires_after(std::chrono::milliseconds(1));
    send_timer_.async_wait(boost::bind(&Server::handleSendTimer, this, boost::asio::placeholders::error));
}

void Server::handleSendTimer(const boost::system::error_code& error) {
    if (!error) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        if (!send_queue_.empty()) {
            std::string message = send_queue_.front();
            send_queue_.pop();
            for (const auto& client : clients_) {
                send_to_client(message, client.second.getEndpoint());
            }
        }
        // Restart the timer
        startSendTimer();
    } else {
        std::cerr << "[ERROR] Timer error: " << error.message() << std::endl;
    }
}

void Server::sendHeartbeatMessage() {
    std::ostringstream message;
    message << static_cast<uint8_t>(Network::PacketType::HEARTBEAT) << ";";
    message << clients_.size() << ";"; // Number of clients
    for (const auto& client : clients_) {
        send_to_client(message.str(), client.second.getEndpoint());
    }
}

void Server::regulate_receive()
{
    receive_timer_.expires_after(std::chrono::milliseconds(10)); // Set the interval to 10 milliseconds
    receive_timer_.async_wait(boost::bind(&Server::startReceive, this));
}

