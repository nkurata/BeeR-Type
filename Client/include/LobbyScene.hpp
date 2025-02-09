#ifndef LOBBY_SCENE_HPP
#define LOBBY_SCENE_HPP

#include "Sprite.hpp"
#include "Scene.hpp"
#include "Packet.hpp"
#include <unordered_map>
#include <optional>

class LobbyScene : public Scene {
public:
    LobbyScene(sf::RenderWindow& window, PacketData& packetData);
    ~LobbyScene();

    void init() override;
    void update(float deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    void handleInput(sf::RenderWindow& window) override;
    void cleanup(sf::RenderWindow& window) override;

    void createSprites();

    void handleKeyPress(sf::Keyboard::Key key, sf::RenderWindow& window);
    void handleMousePressed(sf::RenderWindow& window);
    void initSpriteElement(SpriteElement& element, SpriteType type, float x, float y, int id, int score);
    bool checkGameStart();
    void setPacketData(const PacketData& packetData) const;
    std::optional<Network::PacketType> sendOverPackets();

    sf::RenderWindow* getWindow() override { return &window_; }

private:
    std::unordered_map<SpriteType, sf::Texture> textures_;
    std::unordered_map<int, SpriteElement> scenes_;
    sf::RenderWindow& window_;
    bool startButtonClicked = false;
    PacketData& packetData_;
    std::optional<Network::PacketType> newAction_;
    sf::Font font_;
};

#endif // LOBBY_SCENE_HPP
