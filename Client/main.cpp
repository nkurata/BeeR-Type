#include "Client.hpp"

Client* global_client = nullptr;

void signalHandler(int signum) {
    if (global_client) {
        std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
        global_client->stop();
        global_client->sendExitPacket();
    }
}

int main(int ac, char **av)
{
    if (ac != 4) {
        std::cerr << "Usage: " << av[0] << " <host> <server-port> <client-port>" << std::endl;
        return 84;
    }

    std::string host = av[1];
    short server_port = std::stoi(av[2]);
    short client_port = std::stoi(av[3]);

    try {
        boost::asio::io_context io_context;
        Client client(io_context, host, server_port, client_port);

        global_client = &client;
        std::signal(SIGINT, signalHandler);
        client.main_loop();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

