#include "Client.hpp"

using boost::asio::ip::udp;

Client::Client(boost::asio::io_context& ioContext, const std::string& host, short serverPort, short clientPort)
    : socket_(ioContext, udp::endpoint(udp::v4(), clientPort)),
      io_context_(ioContext),
      window_(sf::VideoMode(1280, 720), "R-Type Client"),
      send_timer_(ioContext),
      heartbeat_timer_(ioContext),
      last_heartbeat_time_(std::chrono::high_resolution_clock::now()),
      last_response_time_(std::chrono::high_resolution_clock::now())
{
    udp::resolver resolver(ioContext);
    udp::resolver::query query(udp::v4(), host, std::to_string(serverPort));
    server_endpoint_ = *resolver.resolve(query).begin();
    std::cout << "Connected to " << host << ":" << serverPort << " from client port " << clientPort << std::endl;

    startReceive();
    startSendTimer();
    schedulePing();
    receive_thread_ = std::thread(&Client::runReceive, this);
}

Client::~Client()
{
    stop();
}

void Client::stop()
{
    // Close the SFML window
    if (window_.isOpen()) {
        window_.close();
    }

    // stop Boost.Asio IO context
    io_context_.stop();

    // stop timers
    send_timer_.cancel();
    heartbeat_timer_.cancel();

    // Close the socket
    if (socket_.is_open()) {
        socket_.close();
    }

    // Ensure the receive thread stops
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
}

void Client::send(const std::string& message)
{
    packets_sent_++;
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

void Client::handleReceive(const boost::system::error_code& error, std::size_t bytesTransferred)
{
    if (!error || error == boost::asio::error::message_size) {
        mutex_.lock();
        packets_received_++;
        received_data_.assign(recv_buffer_.data(), bytesTransferred);
        parseMessage();
        startReceive();
    } else {
        std::cerr << "[ERROR] Error receiving: " << error.message() << std::endl;
        startReceive();
    }
}

double Client::resetPacketLoss()
{
    static auto lastResetTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - lastResetTime;

    if (elapsed.count() >= 10.0) {
        lag_meter_.packets_lost_ = 0;
        lastResetTime = now;
    }

    double packetLoss = 0.0;
    double totalPackets = packets_sent_ + packets_received_;
    if (totalPackets > 0) {
        packetLoss = (lag_meter_.packets_lost_ / totalPackets) * 100.0;
    }
    return std::max(0.0, packetLoss);
}

void Client::handleSend(const boost::system::error_code& error, std::size_t bytesTransferred)
{
    if (!error) {
        // std::cout << "[DEBUG] Message sent." << std::endl;
    } else {
        lag_meter_.packets_lost_++;
        std::cerr << "[ERROR] Error sending: " << error.message() << std::endl;
    }
}

void Client::runReceive()
{
    io_context_.run();
}

std::string Client::createPacket(Network::PacketType type)
{
    Network::Packet packet;
    packet.type = type;
    std::string packetStr;
    packetStr.push_back(static_cast<uint8_t>(type));
    return packetStr;
}

void Client::startSendTimer(){
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

std::vector<std::string> Client::deserialisePacket(std::string packetInside)
{
    std::vector<std::string> elements;
    std::stringstream ss(packetInside);
    std::string segment;

    while (std::getline(ss, segment, ';')) {
        elements.push_back(segment);
    }
    if (elements.size() != 6) {
        std::cerr << "[ERROR] Invalid packet format" << std::endl;
        return {};
    }
    return elements;
}

void Client::parseMessage()
{
    if (received_data_.empty()) {
        std::cerr << "[ERROR] Empty packet data." << std::endl;
        return;
    }

    uint8_t packetType = static_cast<uint8_t>(received_data_[0]);
    std::vector<std::string> elements = deserialisePacket(received_data_.substr(2));
    if (elements.empty()) {
        std::cerr << "[ERROR] Failed to parse packet data." << std::endl;
        return;
    }

    if (packetType == static_cast<uint8_t>(Network::PacketType::HEARTBEAT)) {
        handleHeartBeat();
        awaiting_ = false;
        last_response_time_ = std::chrono::high_resolution_clock::now();
        return;
    }

    try {
        packet_data_.action = static_cast<int>(packetType);
        packet_data_.server_id = std::stoi(elements[0]);
        packet_data_.new_x = std::stof(elements[1]);
        packet_data_.new_y = std::stof(elements[2]);
        packet_data_.new_vx = std::stof(elements[3]);
        packet_data_.new_vy = std::stof(elements[4]);

        int packetnb = std::stoi(elements[5]);

        if (packetnb - last_packetnb_ != 1) {
            lag_meter_.packets_lost_ = packetnb - last_packetnb_ - 1;
            std::cerr << "[WARNING] Packets lost: " << lag_meter_.packets_lost_ << std::endl;
        }

        last_packetnb_ = packetnb;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to parse packet data: " << e.what() << std::endl;
    }
}

void Client::schedulePing()
{
    heartbeat_timer_.expires_after(std::chrono::seconds(2));
    heartbeat_timer_.async_wait([this](const boost::system::error_code& ec) {
        if (!ec) {
            sendHeartBeat();
            checkTimeout();

            double packet_loss = resetPacketLoss();
            lag_meter_.packets_lost_ = packet_loss;
            schedulePing();
        }
    });
}

void Client::sendHeartBeat()
{
    if (!awaiting_) {
        heartbeat_start_time_ = std::chrono::high_resolution_clock::now();
        send_queue_.push(createPacket(Network::PacketType::HEARTBEAT));
        awaiting_ = true;
    }
}

void Client::handleHeartBeat()
{
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> latency = end - heartbeat_start_time_;

    lag_meter_.ping = latency.count() / 1000.0;
}

void Client::checkTimeout()
{
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = now - last_response_time_;

    if (awaiting_ && elapsed.count() > 5000) { // 5000ms timeout
        std::cerr << "[WARNING] Lost Signal." << std::endl;
        awaiting_ = false;
    }
}

int Client::main_loop()
{
    send_queue_.push(createPacket(Network::PacketType::REQCONNECT));
    bool isGameStarted = false;
    LobbyScene lobbyScene(this->window_, packet_data_);
    GameScene gameScene(this->window_, packet_data_, lag_meter_);

    while (this->window_.isOpen()) {
        float deltaTime = game_clock_.restart().asSeconds();

        if (packet_data_.action == 30) {
            std::cerr << "[ERROR] Game already started. Exiting..." << std::endl;
            return 1;
        }

        if (!isGameStarted && this->window_.isOpen()) {
            lobbyScene.setPacketData(packet_data_);
            lobbyScene.update(deltaTime, this->window_);
            auto newAction = lobbyScene.sendOverPackets();
            if (newAction.has_value()) {
                send_queue_.push(createPacket(newAction.value()));
            }
            isGameStarted = lobbyScene.checkGameStart();
        }

        if (isGameStarted && this->window_.isOpen()) {
            gameScene.setPacketData(packet_data_);
            gameScene.setLagMeter(lag_meter_);
            gameScene.update(deltaTime, this->window_);
            auto newAction = gameScene.sendOverPackets();
            if (newAction.has_value()) {
                send_queue_.push(createPacket(newAction.value()));
            }
        }
        if (!this->window_.isOpen()) {
            break;
        }
        mutex_.unlock();
    }
    sendExitPacket();
    return 0;
}
