#include "LobbyScene.hpp"
#include "Client.hpp"

LobbyScene::LobbyScene(sf::RenderWindow& window, Client& client)
    : Scene(window, client)
{
    loadAssets();
}

LobbyScene::~LobbyScene() {
    player_texts_.clear();
    scenes_.clear();
    buttons_.clear();
    players_.clear();
}

void LobbyScene::loadAssets() {
    if (!font_.loadFromFile("../assets/font.otf")) {
        std::cerr << "[ERROR] Failed to load font" << std::endl;
    }
    textures_[SpriteType::LobbyBackground].loadFromFile("../assets/lobby_background.png");
    textures_[SpriteType::GameStart1].loadFromFile("../assets/play_button.png");
    textures_[SpriteType::GameStart2].loadFromFile("../assets/play_button.png");
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

    scenes_[-100] = backgroundElement;

    // Position start buttons in the middle of the screen
    SpriteElement buttonElement1;
    buttonElement1.sprite.setTexture(textures_[SpriteType::GameStart1]);
    buttonElement1.sprite.setPosition((window.getSize().x / 2) - textures_[SpriteType::GameStart1].getSize().x - 10, 20);
    buttonElement1.id = -101;

    SpriteElement buttonElement2;
    buttonElement2.sprite.setTexture(textures_[SpriteType::GameStart2]);
    buttonElement2.sprite.setPosition((window.getSize().x / 2) + 10, 20);
    buttonElement2.id = -102;

    buttons_[-101] = buttonElement1;
    buttons_[-102] = buttonElement2;

    // Add player sprites and texts at the bottom of the screen
    for (int i = 0; i < client.getNumClients(); ++i) {
        SpriteElement playerElement;
        playerElement.sprite.setTexture(textures_[SpriteType::PlayerIcon]);
        playerElement.sprite.setScale(0.40f, 0.40f);
        playerElement.sprite.setPosition(50 + i * (playerElement.sprite.getGlobalBounds().width + 20), window.getSize().y - playerElement.sprite.getGlobalBounds().height - 50);
        playerElement.id = i;
        players_[i] = playerElement;

        sf::Text playerText;
        playerText.setFont(font_);
        playerText.setString("Player " + std::to_string(i + 1));
        playerText.setCharacterSize(24);
        playerText.setFillColor(sf::Color::White);
        // Align text horizontally with the middle of the icon
        float textX = playerElement.sprite.getPosition().x + (playerElement.sprite.getGlobalBounds().width / 2) - (playerText.getGlobalBounds().width / 2);
        playerText.setPosition(textX, playerElement.sprite.getPosition().y - playerText.getGlobalBounds().height - 10);
        player_texts_.push_back(playerText);
    }
}

void LobbyScene::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (buttons_.find(-101) != buttons_.end() && buttons_[-101].sprite.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                std::cout << "Game starting..." << std::endl;
                client.send_queue_.push(client.createPacket(Network::PacketType::GAME_START));
                buttons_.erase(-101);
            } if (buttons_.find(-102) != buttons_.end() && buttons_[-102].sprite.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                std::cout << "Game starting..." << std::endl;
                client.send_queue_.push(client.createPacket(Network::PacketType::GAME_START));
                buttons_.erase(-102);
            }
        }
    }
}

void LobbyScene::update() {
    numClients_ = client.getNumClients();
    updatePing(); // Update the ping text
    initLobbySprites();
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
    renderOverlay(); // Render the ping and chat log overlay
    window.display();
}