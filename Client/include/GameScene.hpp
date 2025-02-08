#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "Scene.hpp"
#include "Sprite.hpp"
#include "Client.hpp"
#include "Registry.hpp"
#include "PositionSystem.hpp"
#include "ProjectileSystem.hpp"
#include "DrawSystem.hpp"
#include "RTYPEPlayer.hpp"
#include "RTYPEEnemy.hpp"
#include "RTYPEBoss.hpp"
#include "RTYPEBullet.hpp"
#include <unordered_map>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>

class GameScene : public Scene {
public:
    GameScene(sf::RenderWindow& window, Client& client);
    ~GameScene();
    void run(sf::RenderWindow& window, Client& client);
    void processEvents() override;
    void update() override;
    void render() override;

private:
    void createSprite();
    void initBackground();
    void handleKeyPress(sf::Keyboard::Key key);
    void handleKeyUnpress(sf::Keyboard::Key key);
    void handleServerActions();

    std::unordered_map<int, std::unique_ptr<Player>> players_;
    std::unordered_map<int, std::unique_ptr<Enemy>> enemies_;
    std::unordered_map<int, std::unique_ptr<Boss>> bosses_;
    Registry registry_;
    sf::Sprite backgroundSprite;
    sf::Texture backgroundTexture;
    std::vector<sf::Text> gameTexts_;
    sf::RenderWindow& window_;
    Client& client_;
};

#endif // GAME_SCENE_HPP
