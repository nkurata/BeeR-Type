#include "GameScene.hpp"
#include "Client.hpp"
#include <iostream> // Add for debug output

GameScene::GameScene(sf::RenderWindow& window, Client& client)
    : Scene(window, client)
{
    std::cout << "[DEBUG] GameScene constructor called" << std::endl;
    loadAssets();
    initGameSprites();
    LoadSound();
    std::cout << "[DEBUG] GameScene constructor finished" << std::endl;
}

GameScene::~GameScene()
{
    std::cout << "[DEBUG] GameScene destructor called" << std::endl;
    sound_background_.stop();
    std::cout << "[DEBUG] GameScene destructor finished" << std::endl;
}

void GameScene::loadAssets() {
    std::cout << "[DEBUG] Loading assets" << std::endl;
    if (!font_.loadFromFile("../assets/font.otf")) {
        std::cerr << "[ERROR] Failed to load font" << std::endl;
    }
    if (!textures_[SpriteType::BackgroundSprite].loadFromFile("../assets/background.png")) {
        std::cerr << "[ERROR] Failed to load background texture" << std::endl;
    }
    if (!textures_[SpriteType::PlayerSprite].loadFromFile("../assets/player.png")) {
        std::cerr << "[ERROR] Failed to load player texture" << std::endl;
    }
    if (!textures_[SpriteType::EnemySprite].loadFromFile("../assets/enemy.png")) {
        std::cerr << "[ERROR] Failed to load enemy texture" << std::endl;
    }
    if (!textures_[SpriteType::BulletSprite].loadFromFile("../assets/bullet.png")) {
        std::cerr << "[ERROR] Failed to load bullet texture" << std::endl;
    }
    if (!textures_[SpriteType::BossSprite].loadFromFile("../assets/boss.png")) {
        std::cerr << "[ERROR] Failed to load boss texture" << std::endl;
    }
    std::cout << "[DEBUG] Assets loaded" << std::endl;
}

void GameScene::createSprite(int action, int server_id, float new_x, float new_y)
{
    std::cout << "[DEBUG] Creating sprite with action: " << action << ", server_id: " << server_id << std::endl;
    SpriteElement spriteElement;
    SpriteType spriteType;

    if (action == 22) { // Create enemy sprite
        spriteType = SpriteType::EnemySprite;
        enemies_[server_id] = spriteElement;
    } else if (action == 23) { // Create boss sprite
        spriteType = SpriteType::BossSprite;
        bosses_[server_id] = spriteElement;
    } else if (action == 24) { // Create player sprite
        spriteType = SpriteType::PlayerSprite;
        players_[server_id] = spriteElement;
    } else if (action == 25) { // Create bullet sprite
        spriteType = SpriteType::BulletSprite;
        bullets_[server_id] = spriteElement;
    } else if (action == 26) { // Create background sprite
        spriteType = SpriteType::BackgroundSprite;
        gameElements_[-100] = spriteElement;
    } else {
        return;
    }

    spriteElement.sprite.setTexture(textures_[spriteType]);
    spriteElement.sprite.setPosition(new_x, new_y);
    spriteElement.id = server_id;
    std::cout << "[DEBUG] Sprite created" << std::endl;
}

void GameScene::adjustVolume(float change)
{
    float newVolume = sound_background_.getVolume() + change;
    if (newVolume < 0) newVolume = 0;
    if (newVolume > 100) newVolume = 100;
    sound_background_.setVolume(newVolume);
}

void GameScene::LoadSound()
{
    std::cout << "[DEBUG] Loading sounds" << std::endl;
    if (!buffer_background_.loadFromFile("../assets/sound.wav")) {
        std::cerr << "[ERROR] Failed to load background sound" << std::endl;
    }
    if (!buffer_shoot_.loadFromFile("../assets/shoot.wav")) {
        std::cerr << "[ERROR] Failed to load shoot sound" << std::endl;
    }
    sound_background_.setBuffer(buffer_background_);
    sound_shoot_.setBuffer(buffer_shoot_);
    sound_shoot_.setVolume(50);
    sound_background_.setVolume(50);
    sound_background_.setLoop(true);
    sound_background_.play();
    std::cout << "[DEBUG] Sounds loaded" << std::endl;
}

void GameScene::initGameSprites() {
    std::cout << "[DEBUG] Initializing game sprites" << std::endl;
    players_.clear();
    enemies_.clear();
    bullets_.clear();
    bosses_.clear();
    gameTexts_.clear(); // Clear previous texts

    SpriteElement backgroundElement;
    backgroundElement.sprite.setTexture(textures_[SpriteType::BackgroundSprite]);
    backgroundElement.sprite.setPosition(0, 0);
    // Scale the background to fit the window size
    float scaleX = static_cast<float>(window.getSize().x) / backgroundElement.sprite.getTexture()->getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / backgroundElement.sprite.getTexture()->getSize().y;
    backgroundElement.sprite.setScale(scaleX, scaleY);
    backgroundElement.id = -100;

    gameElements_[-100] = backgroundElement;

    // Spawn players on the left side of the screen in a line
    for (int i = 0; i < client.getNumClients(); ++i) {
        SpriteElement playerElement;
        playerElement.sprite.setTexture(textures_[SpriteType::PlayerSprite]);
        playerElement.sprite.setPosition(50, 100 + i * 100);
        playerElement.id = i;
        players_[i] = playerElement;
    }

    // Spawn an enemy in the middle of the screen for testing
    SpriteElement enemyElement;
    enemyElement.sprite.setTexture(textures_[SpriteType::EnemySprite]);
    enemyElement.sprite.setPosition(window.getSize().x / 2, window.getSize().y / 2);
    enemyElement.id = 1;
    enemies_[1] = enemyElement;
    std::cout << "[DEBUG] Game sprites initialized" << std::endl;
}

void GameScene::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        } else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (!players_.empty()) {
                client.send(client.createMousePacket(Network::PacketType::PLAYER_SHOOT, mousePos.x, mousePos.y));
            }
        }
    }
}

void GameScene::handleKeyPress(sf::Keyboard::Key key)
{
    switch (key) {
        case sf::Keyboard::Right:
            client.send(client.createPacket(Network::PacketType::PLAYER_RIGHT));
            break;

        case sf::Keyboard::Left:
            client.send(client.createPacket(Network::PacketType::PLAYER_LEFT));
            break;

        case sf::Keyboard::Up:
            client.send(client.createPacket(Network::PacketType::PLAYER_UP));
            break;

        case sf::Keyboard::Down:
            client.send(client.createPacket(Network::PacketType::PLAYER_DOWN));
            break;

        case sf::Keyboard::M:
            client.send(client.createPacket(Network::PacketType::OPEN_MENU));
            break;

        case sf::Keyboard::Space:
            client.send(client.createPacket(Network::PacketType::PLAYER_SHOOT));
            sound_shoot_.play();
            break;

        case sf::Keyboard::Escape:
            client.send(client.createPacket(Network::PacketType::GAME_END));
            break;

        case sf::Keyboard::Num1:
            adjustVolume(-5);
            break;

        case sf::Keyboard::Num2:
            adjustVolume(5);
            break;

        default:
            break;
    }
}

void GameScene::handleKeyUnpress(sf::Keyboard::Key key)
{
    switch (key) {
        case sf::Keyboard::Right:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP_RIGHT));
            break;

        case sf::Keyboard::Left:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP_LEFT));
            break;

        case sf::Keyboard::Up:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP_UP));
            break;

        case sf::Keyboard::Down:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP_DOWN));
            break;

        default:
            break;
    }
}

void GameScene::update() {
    for (auto& [id, enemy] : enemies_) {
        enemy.sprite.move(-1, 0);
    }
}

void GameScene::render() {
    window.clear();
    for (const auto& element : gameElements_) {
        window.draw(element.second.sprite);
    }
    for (const auto& player : players_) {
        window.draw(player.second.sprite);
    }
    for (const auto& enemy : enemies_) {
        window.draw(enemy.second.sprite);
    }
    for (const auto& bullet : bullets_) {
        window.draw(bullet.second.sprite);
    }
    for (const auto& boss : bosses_) {
        window.draw(boss.second.sprite);
    }
    for (const auto& text : gameTexts_) {
        window.draw(text);
    }
    window.display();
}
