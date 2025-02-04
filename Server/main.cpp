/*
** EPITECH PROJECT, 2024
** R-Type [WSL: Ubuntu]
** File description:
** main
*/

#include "Server.hpp"
#include "Errors/Throws.hpp"
#include "Packet.hpp"
#include "ThreadSafeQueue.hpp"
#include "PacketHandler.hpp"
#include "Game.hpp"
#include <dlfcn.h>

short parsePort(int ac, char **av)
{
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " <port>" << std::endl;
        throw RType::InvalidPortException("Invalid number of arguments");
    }
    try {
        return std::stoi(av[1]);
    } catch (const std::exception& e) {
        throw RType::InvalidPortException("Invalid port number");
    }
}


typedef IGame *createGame_t(Server &server);

IGame *getGameInstance(const std::string &path, Server &server)
{
    void *handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw std::runtime_error(dlerror());
    }
    auto *createGame = (createGame_t *)dlsym(handle, "getRTypeGame");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        throw std::runtime_error(dlsym_error);
    } if (!createGame) {
        throw std::runtime_error(dlerror());
    } if (handle) {
        dlclose(handle);
    }
    return createGame(server);
}

void runServer(short port) {
    try {
        boost::asio::io_context io_context;
        ThreadSafeQueue<Network::Packet> packetQueue;

        Server server(io_context, port, packetQueue);

        IGame *game = getGameInstance("R-Type.so", server);
        if (!game) {
            throw RType::UnknownException("Failed to load game instance");
        }
        // server.setGameState(&game);

        Network::PacketHandler packetHandler(packetQueue, server, dynamic_cast<Game &>(*game));
        packetHandler.start();

        std::cout << "Server started\nListening on UDP port " << port << std::endl;

        std::thread serverThread([&io_context] {
            io_context.run();
        });

        if (serverThread.joinable())
            serverThread.join();

        packetHandler.stop();
    } catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::access_denied) {
            throw RType::PermissionDeniedException("Permission denied");
        } else {
            throw;
        }
    }
}

int main(int ac, char **av)
{
    try {
        short port = parsePort(ac, av);
        runServer(port);
    } catch (const RType::NtsException& e) {
        std::cerr << "Exception: " << e.what() << " (Type: " << e.getType() << ")" << std::endl;
    } catch (const std::exception& e) {
        throw RType::StandardException(e.what());
    } catch (...) {
        throw RType::UnknownException("Unknown exception occurred");
    }
    return 0;
}