#ifndef GAME_SCENE_HPP
#define GAME_SCENE_HPP

#include "Scene.hpp"
#include "Client.hpp"
#include <unordered_map>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class GameScene : public Scene {
public:
    GameScene(sf::RenderWindow& window, Client& client);
    ~GameScene(); // Add destructor
    void processEvents() override;
    void update() override;
    void render() override;

private:
    void loadAssets();
    void initGameSprites();
    void createSprite(int action, int server_id, float new_x, float new_y);
    void adjustVolume(float change);
    void LoadSound();
    void handleKeyPress(sf::Keyboard::Key key, sf::RenderWindow& window);

    std::unordered_map<int, SpriteElement> players_;
    std::unordered_map<int, SpriteElement> enemies_;
    std::unordered_map<int, SpriteElement> bullets_;
    std::unordered_map<int, SpriteElement> bosses_;
    std::unordered_map<int, SpriteElement> gameElements_;
    std::unordered_map<SpriteType, sf::Texture> textures_;
    std::vector<sf::Text> gameTexts_;

    sf::SoundBuffer buffer_background_;
    sf::SoundBuffer buffer_shoot_;
    sf::Sound sound_background_;
    sf::Sound sound_shoot_;
};

#endif // GAME_SCENE_HPP
