
#include "Server.hpp"

using boost::asio::ip::udp;

Server::Server(boost::asio::io_context& io_context, short port, ThreadSafeQueue<Network::Packet>& packetQueue)
: socket_(io_context, udp::endpoint(udp::v4(), port)), m_packetQueue(packetQueue), _nbClients(0), game_running(false), send_timer_(io_context), print_timer_(io_context)
{
    startReceive();
    start_send_timer();
    start_print_timer();
}

Server::~Server()
{
    socket_.close();
}

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
    send_queue_.push(message);
}
void Server::startReceive()
{
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&Server::handleReceive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Server::handleReceive(const boost::system::error_code &error, std::size_t bytes_transferred)
{
    if (!error || error == boost::asio::error::message_size) {
        bytes_recieved_5_seconds += bytes_transferred;
        std::string received_data(recv_buffer_.data(), bytes_transferred);
        Network::Packet packet;
        packet.type = deserializePacket(received_data).type;
        m_packetQueue.push(packet);
    } else {
        std::cerr << "[ERROR] Error receiving: " << error.message() << std::endl;
        if (error == boost::asio::error::operation_aborted) {
            return;
        }
    }
    startReceive();
}

Network::Packet Server::deserializePacket(const std::string& packet_str)
{
    Network::Packet packet;
    packet.type = static_cast<Network::PacketType>(packet_str[0]);
    return packet;
}

std::string Server::createPacket(const Network::PacketType& type, const std::string& data)
{
    std::string packet_str;
    std::string packet_data = data.empty() ? "-1;-1;-1;-1;-1" : data;
    // std::cout << "[DEBUG] Creating packet with type: " << static_cast<int>(type) << " and data: " << packet_data << std::endl;

    packet_str.push_back(static_cast<uint8_t>(type));
    packet_str.push_back(static_cast<uint8_t>(';'));
    for (char c : packet_data) {
        packet_str.push_back(static_cast<uint8_t>(c));
    }
    packet_str.push_back(static_cast<uint8_t>(';'));
    sequence_number++;
    packet_str += std::to_string(sequence_number);
    std::cout << "[DEBUG] Packet created: " << static_cast<int>(type) << " - " << packet_str << std::endl;
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
    if (game_running)
        send_to_client(createPacket(Network::PacketType::GAME_STARTED, ""), client_endpoint);
    else
        send_to_client(createPacket(Network::PacketType::GAME_NOT_STARTED, ""), client_endpoint);
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
                std::cout << "[DEBUG] Client " << data.id << " disconnected." << std::endl;

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

void Server::sendPong()
{
    send_queue_.push(createPacket(Network::PacketType::HEARTBEAT, ""));
}

void Server::start_print_timer()
{
    print_timer_.expires_after(std::chrono::seconds(5));
    print_timer_.async_wait(boost::bind(&Server::handle_print_timer, this, boost::asio::placeholders::error));
}

void Server::handle_print_timer(const boost::system::error_code& error)
{
    if (!error) {
        std::cout << bytes_recieved_5_seconds << " bytes received in the last 5 seconds." << std::endl;
        bytes_recieved_5_seconds = 0;
    } else {
        std::cerr << "[DEBUG] Print timer error: " << error.message() << std::endl;
    }
    start_print_timer();
}

void Server::start_send_timer()
{
    send_timer_.expires_after(std::chrono::milliseconds(1));
    send_timer_.async_wait(boost::bind(&Server::handle_send_timer, this, boost::asio::placeholders::error));
}

void Server::handle_send_timer(const boost::system::error_code& error)
{
    if (!error) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        if (!send_queue_.empty()) {
            std::string message = send_queue_.front();
            send_queue_.pop();
            for (const auto& client : clients_) {
                send_to_client(message, client.second.getEndpoint());
            }
        }
        start_send_timer();
    } else {
        std::cerr << "[DEBUG] Timer error: " << error.message() << std::endl;
    }
}
