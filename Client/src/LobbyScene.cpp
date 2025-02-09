#include "LobbyScene.hpp"
#include "Client.hpp"

LobbyScene::LobbyScene(sf::RenderWindow& window, PacketData& packetData) : window_(window), packetData_(packetData), startButtonClicked(false), newAction_(std::nullopt)
{
    LobbyScene::init();
}

LobbyScene::~LobbyScene() {
    LobbyScene::cleanup(window_);
}

void LobbyScene::init() {
    if (!textures_[SpriteType::Background].loadFromFile("../assets/lobby_background.png")) {
        std::cerr << "[ERROR] Failed to load background texture" << std::endl;
    }
    if (!textures_[SpriteType::Start_button].loadFromFile("../assets/play_button.png")) {
        std::cerr << "[ERROR] Failed to load start button texture" << std::endl;
    }
    createSprites();
}

void LobbyScene::cleanup(sf::RenderWindow &window) {
    window.clear();
    scenes_.clear();
}

bool LobbyScene::checkGameStart() {
    if (startButtonClicked || packetData_.action == 3) {
        startButtonClicked = false;
        cleanup(window_);
        newAction_ = Network::PacketType::GAME_START;
        return true;
    }
    return false;
}

std::optional<Network::PacketType> LobbyScene::sendOverPackets() {
    auto action = newAction_;
    newAction_.reset();
    return action;
}

void LobbyScene::setPacketData(const PacketData &packetData) const {
    packetData_ = packetData;
}

void LobbyScene::handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                cleanup(window);
            window.close();
            break;
            case sf::Event::MouseButtonPressed: {
                handleMousePressed(window);
                break;
            }
            default:
                break;
        }
    }
}

void LobbyScene::handleMousePressed(sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    for (const auto& [id, element] : scenes_) {
        if (element.sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
            if (element.id == 0) {
                startButtonClicked = true;
                newAction_ = Network::PacketType::GAME_START;
                break;
            }
        }
    }
}

void LobbyScene::createSprites() {
    scenes_.clear();
    SpriteElement backgroundElement;
    initSpriteElement(backgroundElement, SpriteType::Background, 0, 0, 1, 0);

    SpriteElement startButtonElement;
    initSpriteElement(startButtonElement, SpriteType::Start_button,
                      window_.getSize().x / 2 - textures_[SpriteType::Start_button].getSize().x / 2,
                      window_.getSize().y / 2 - textures_[SpriteType::Start_button].getSize().y / 2, 0, 0);

    scenes_[startButtonElement.id] = startButtonElement;
    scenes_[backgroundElement.id] = backgroundElement;
    // SpriteElement playerElement;
    // initSpriteElement(playerElement, SpriteType::Player, 100, 100, 3, 0);

}

void LobbyScene::initSpriteElement(SpriteElement &element, SpriteType type, float x, float y, int id, int score) {
    element.sprite.setTexture(textures_[type]);
    element.sprite.setPosition(x, y);
    element.id = id;
}


void LobbyScene::update(float deltaTime, sf::RenderWindow& window) {
    if (startButtonClicked) {
        cleanup(window);
    }
    handleInput(window);
    render(window);
}

void LobbyScene::render(sf::RenderWindow& window) {
    window.clear();
    for (const auto& [id, elmt] : scenes_) {
        window.draw(elmt.sprite);
    }
    window.display();
}