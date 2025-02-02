#ifndef SCENE_HPP
#define SCENE_HPP

#include "Sprite.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// Forward declaration of Client class
class Client;

enum class SceneType {
    Lobby,
    Game
};

class Scene {
public:
    Scene(sf::RenderWindow& window, Client& client);
    virtual ~Scene(); // Add virtual destructor

    virtual void processEvents() = 0;
    virtual void update() = 0;
    virtual void render() = 0;

    void updatePing();
    void addChatLog(const std::string& message);
    void renderOverlay();

protected:
    sf::RenderWindow& window;
    Client& client;
    int numClients_;
    int current_ping_ = 0;
    std::vector<std::string> chat_log_;
    const size_t max_chat_messages_ = 10;
    sf::Font font_;
    sf::Text ping_text_;
    sf::Text chat_text_;
};

#endif // SCENE_HPP