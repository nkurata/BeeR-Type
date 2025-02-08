#ifndef GAMELOADER_HPP
#define GAMELOADER_HPP

#include <string>
#include <stdexcept>
#include <dlfcn.h>
#include "Server.hpp"
#include "IGame.hpp"

class GameLoader {
public:
    explicit GameLoader(const std::string& libraryPath) {
        handle_ = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (!handle_) {
            throw std::runtime_error(dlerror());
        }
    }
    ~GameLoader() {
        if (handle_) {
            dlclose(handle_);
        }
    }

    void* getSymbol(const std::string &symbolName)
    {
        void *symbol = dlsym(handle_, symbolName.c_str());
        const char *error = dlerror();
        if (error) {
            throw std::runtime_error(error);
        }
        return symbol;
    }

    static IGame* getGameInstance(const std::string &gameName, Server &server) {
        try {
            GameLoader loader(gameName);
            using create_t = IGame* (*)(Server &);
            auto create = reinterpret_cast<create_t>(loader.getSymbol("createGame"));
            return create(server);
        } catch (const std::exception &e) {
            throw std::runtime_error(std::string("Failed to load game: ") + e.what());
        }
    }

private:
    void* handle_;
};

#endif //GAMELOADER_HPP