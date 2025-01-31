#include "LobbyScene.hpp"
#include "Client.hpp"

LobbyScene::LobbyScene(sf::RenderWindow& window, Client& client)
    : Scene(window, client)
{
    loadAssets(); // Load textures when the scene is created
    initLobbySprites(); // Initialize lobby sprites
}

void LobbyScene::loadAssets() {
    if (!font_.loadFromFile("../assets/arial.ttf")) {
        std::cerr << "[ERROR] Failed to load font" << std::endl;
    }
    textures_[SpriteType::LobbyBackground].loadFromFile("../assets/lobby_background.jpg");
    textures_[SpriteType::StartButton].loadFromFile("../assets/play_button.png");
    textures_[SpriteType::PlayerIcon].loadFromFile("../assets/player_icon.png");
}

void LobbyScene::initLobbySprites() {
    scenes_.clear();
    player_texts_.clear(); // Clear previous texts

    SpriteElement backgroundElement;
    backgroundElement.sprite.setTexture(textures_[SpriteType::LobbyBackground]);
    backgroundElement.sprite.setPosition(0, 0);
    // Scale the background to fit the window size
    float scaleX = static_cast<float>(window.getSize().x) / backgroundElement.sprite.getTexture()->getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / backgroundElement.sprite.getTexture()->getSize().y;
    backgroundElement.sprite.setScale(scaleX, scaleY);
    backgroundElement.id = -100;

    SpriteElement buttonElement;
    buttonElement.sprite.setTexture(textures_[SpriteType::StartButton]);
    buttonElement.sprite.setPosition(window.getSize().x - textures_[SpriteType::StartButton].getSize().x - 20, window.getSize().y - textures_[SpriteType::StartButton].getSize().y - 20);
    buttonElement.id = -101;

    scenes_[-100] = backgroundElement;
    buttons_[-101] = buttonElement;

    // Add player sprites and texts on the left side of the screen
    for (int i = 0; i < numClients_; ++i) {
        SpriteElement playerElement;
        playerElement.sprite.setTexture(textures_[SpriteType::PlayerIcon]);
        playerElement.sprite.setScale(0.40f, 0.40f);
        playerElement.sprite.setPosition(50, 100 + i * 100);
        playerElement.id = i;
        players_[i] = playerElement;

        sf::Text playerText;
        playerText.setFont(font_);
        playerText.setString("Player " + std::to_string(i + 1));
        playerText.setCharacterSize(24);
        playerText.setFillColor(sf::Color::White);
        // Align text vertically with the middle of the icon
        float textY = playerElement.sprite.getPosition().y + (playerElement.sprite.getGlobalBounds().height / 2) - (playerText.getGlobalBounds().height / 2);
        playerText.setPosition(50 + playerElement.sprite.getGlobalBounds().width + 10, textY); 
        player_texts_.push_back(playerText);
    }
}

void LobbyScene::setNumClients(int numClients) {
    numClients_ = numClients;
    initLobbySprites(); // Ensure this is called to update the sprites
}

void LobbyScene::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (buttons_.find(-101) != buttons_.end() && buttons_[-101].sprite.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                client.send(client.createPacket(Network::PacketType::GAME_START)); // Call createPacket correctly
                buttons_.erase(-101);
            }
        }
        // Handle other events...
    }
}

void LobbyScene::update() {
    // Update lobby state...
}

void LobbyScene::render() {
    window.clear();
    // Draw lobby elements...
    for (const auto& scene : scenes_) {
        window.draw(scene.second.sprite);
    }
    for (const auto& button : buttons_) {
        window.draw(button.second.sprite);
    }
    for (const auto& player : players_) {
        window.draw(player.second.sprite);
    }
    for (const auto& text : player_texts_) {
        window.draw(text);
    }
    window.display();
}