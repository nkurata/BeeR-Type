#ifndef LOBBY_SCENE_HPP
#define LOBBY_SCENE_HPP

#include "Scene.hpp"
#include "Client.hpp" // Include Client.hpp to use SpriteElement and SpriteType
#include <unordered_map>
#include "Sprite.hpp"

class LobbyScene : public Scene {
public:
    LobbyScene(sf::RenderWindow& window, Client& client);
    ~LobbyScene(); // Add destructor
    void processEvents() override;
    void update() override;
    void render() override;

private:
    void loadAssets();
    void initLobbySprites();
    int numClients_;
    int current_ping_ = 0;

    std::unordered_map<int, SpriteElement> scenes_;
    std::unordered_map<int, SpriteElement> buttons_;
    std::unordered_map<int, SpriteElement> players_;
    std::vector<sf::Text> player_texts_;
    std::unordered_map<SpriteType, sf::Texture> textures_;
};

#endif // LOBBY_SCENE_HPP