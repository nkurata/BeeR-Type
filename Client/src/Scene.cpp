#include "Scene.hpp"
#include "Client.hpp"

Scene::Scene(sf::RenderWindow& window, Client& client)
    : window(window), client(client), numClients_(0) {
    if (!font_.loadFromFile("../assets/font.otf")) {
        std::cerr << "[ERROR] Failed to load font" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    ping_text_.setFont(font_);
    ping_text_.setCharacterSize(24);
    ping_text_.setFillColor(sf::Color::White);
    ping_text_.setPosition(10, 10);

    chat_text_.setFont(font_);
    chat_text_.setCharacterSize(18);
    chat_text_.setFillColor(sf::Color::White);
    chat_text_.setPosition(10, 50);
}

void Scene::updatePing() {
    current_ping_ = client.getPing();
    ping_text_.setString("Ping: " + std::to_string(current_ping_) + " ms");
}

void Scene::addChatLog(const std::string& message) {
    chat_log_.push_back(message);
    if (chat_log_.size() > 10) {
        chat_log_.erase(chat_log_.begin());
    }
    std::string chat_display;
    for (const auto& msg : chat_log_) {
        chat_display += msg + "\n";
    }
    chat_text_.setString(chat_display);
}

void Scene::renderOverlay() {
    window.draw(ping_text_);
    window.draw(chat_text_);
}

