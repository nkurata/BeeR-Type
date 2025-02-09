#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "Scene.hpp"
#include "Sprite.hpp"
#include "Packet.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <chrono>

class GameScene : public Scene {
public:
    GameScene(sf::RenderWindow& window, PacketData& packetData, LagMeter& LagMeter);
    ~GameScene();

    void init() override;
    void cleanup(sf::RenderWindow& window) override;

    void update(float deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;

    void handleInput(sf::RenderWindow& window) override;
    void handleKeyPress(sf::Keyboard::Key key, sf::RenderWindow& window);
    void handleServerAction();

    void createSprites(SpriteType type, std::unordered_map<int, SpriteElement>& spriteMap);
    void destroySprite(std::unordered_map<int, SpriteElement>& spriteMap);

    void updatePositions();
    void updateSpritePosition();

    void setTextProperties(sf::Text& text, const sf::Font& font, unsigned int size, const sf::Color& color, float x, float y);
    void setUI(sf::RenderWindow& window);
    void updateUI();

    void drawSprites(sf::RenderWindow& window);
    void setStats(sf::RenderWindow& window);

    void setLagMeter(LagMeter& LagMeter);
    void setPacketData(PacketData& packetData);
    std::optional<Network::PacketType> sendOverPackets();

    void resetValues();

    bool checkGameEnd();

    sf::RenderWindow* getWindow() override { return &window_; }

private:
    sf::RenderWindow& window_;
    std::unordered_map<int, SpriteElement> bulletSprites_;

    std::unordered_map<int, bool> playerAliveStatus_;
    std::unordered_map<int, SpriteElement> playerSprites_;
    std::unordered_map<int, sf::Vector2f> server_positions_;

    std::unordered_map<int, SpriteElement> enemySprites_;
    std::unordered_map<int, SpriteElement> bossSprites_;
    std::unordered_map<int, SpriteElement> sceneSprites_;
    std::unordered_map<int, SpriteElement> fastBulletSprites_;

    std::unordered_map<SpriteType, sf::Texture> textures_;

    std::unordered_map<std::string, sf::Text> ui_texts_;
    std::unordered_map<int, sf::Text> uiSprites;

    std::optional<Network::PacketType> newAction_;

    std::unordered_set<int> processedPackets;

    int health_ = 100;
    int score_ = 0;
    sf::Font font_;
    sf::Clock game_clock_;

    PacketData& packet_data_;
    LagMeter& lag_meter_;
};

#endif // GAME_SCENE_HPP