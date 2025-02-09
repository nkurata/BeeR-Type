#ifndef SCENE_HPP
#define SCENE_HPP

#include <SFML/Graphics.hpp>


// Forward declaration of Client class
class Client;

enum class SceneType {
    Lobby,
    Game
};

class Scene {
public:
    virtual ~Scene() = default;

    virtual void init() = 0;
    virtual void update(float deltaTime, sf::RenderWindow& window) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual void handleInput(sf::RenderWindow& window) = 0;
    virtual void cleanup(sf::RenderWindow& window) = 0;

    virtual sf::RenderWindow* getWindow() = 0;
};

#endif // SCENE_HPP