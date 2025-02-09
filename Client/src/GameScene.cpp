#include "GameScene.hpp"
#include <iostream>


GameScene::GameScene(sf::RenderWindow& window, PacketData& packetData, LagMeter& lag_meter) : window_(window), packet_data_(packetData), lag_meter_(lag_meter), game_clock_(), health_(100), score_(0), newAction_(std::nullopt)
{
    init();
    createSprites(SpriteType::Background, sceneSprites_);
    setUI(window_);
    window_.setFramerateLimit(60);
}

GameScene::~GameScene()
{
    cleanup(window_);
}

void GameScene::init()
{
    if (!textures_[SpriteType::Background].loadFromFile("../assets/background.png")) {
        std::cerr << "[ERROR] Failed to load background texture" << std::endl;
    }
    if (!textures_[SpriteType::Bullet].loadFromFile("../assets/bullet.png")) {
        std::cerr << "[ERROR] Failed to load bullet texture" << std::endl;
    }
    if (!textures_[SpriteType::Player].loadFromFile("../assets/player.png")) {
        std::cerr << "[ERROR] Failed to load player texture" << std::endl;
    }
    if (!textures_[SpriteType::Enemy].loadFromFile("../assets/enemy.png")) {
        std::cerr << "[ERROR] Failed to load enemy texture" << std::endl;
    }
    if (!textures_[SpriteType::Boss].loadFromFile("../assets/boss.png")) {
        std::cerr << "[ERROR] Failed to load boss texture" << std::endl;
    }
}

void GameScene::update(float deltaTime, sf::RenderWindow& window)
{
    handleServerAction();
    render(window);
    checkGameEnd();
    handleInput(window);
}

void GameScene::render(sf::RenderWindow& window) {
    window.clear();
    updatePositions();
    setStats(window);
    drawSprites(window);
    window.display();
}

void GameScene::createSprites(SpriteType type, std::unordered_map<int, SpriteElement>& spriteMap)
{
    SpriteElement spriteElement;
    spriteElement.id = packet_data_.server_id;
    spriteElement.sprite.setTexture(textures_[type]);
    spriteElement.sprite.setPosition(packet_data_.new_x, packet_data_.new_y);
    spriteElement.vx = packet_data_.new_vx;
    spriteElement.vy = packet_data_.new_vy;
    spriteElement.hp = 100;
    spriteMap[spriteElement.id] = spriteElement;
}

void GameScene::destroySprite(std::unordered_map<int, SpriteElement>& spriteMap) {
    spriteMap.erase(packet_data_.server_id);
}

void GameScene::updateSpritePosition() {
    if (playerSprites_.find(packet_data_.server_id) != playerSprites_.end()) {
        auto& spriteElement = playerSprites_[packet_data_.server_id];
        server_positions_[spriteElement.id] = sf::Vector2f(packet_data_.new_x, packet_data_.new_y);
        spriteElement.sprite.setPosition(server_positions_[spriteElement.id]);
    }
}

void GameScene::resetValues() {
    packet_data_.action = 0;
    packet_data_.server_id = 0;
    packet_data_.new_x = 0;
    packet_data_.new_y = 0;
    packet_data_.new_vx = 0;
    packet_data_.new_vy = 0;
}

void GameScene::handleServerAction() {
    switch (packet_data_.action) {
        case static_cast<int>(Network::PacketType::CREATE_ENEMY): // Enemy
            createSprites(SpriteType::Enemy, enemySprites_);
            std::cout << "Enemy created" << std::endl;
            break;
        case static_cast<int>(Network::PacketType::CREATE_BOSS): // Boss
            createSprites(SpriteType::Boss, bossSprites_);
            break;
        case static_cast<int>(Network::PacketType::CREATE_PLAYER): // Player
            createSprites(SpriteType::Player, playerSprites_);
            playerAliveStatus_[packet_data_.server_id] = true;
            break;
        case static_cast<int>(Network::PacketType::CREATE_BULLET): // Bullet
            createSprites(SpriteType::Bullet, bulletSprites_);
            break;
        case static_cast<int>(Network::PacketType::CHANGE): // Update Sprite Position
            updateSpritePosition();
            break;
        case static_cast<int>(Network::PacketType::UI_UPDATE): // Update UI
            updateUI();
            break;
        case static_cast<int>(Network::PacketType::DESTROY_PLAYER): // Destroy Player
            destroySprite(playerSprites_);
            playerAliveStatus_[packet_data_.server_id] = false;
            break;
        case static_cast<int>(Network::PacketType::DESTROY_ENEMY): // Destroy Enemy
            destroySprite(enemySprites_);
            break;
        case static_cast<int>(Network::PacketType::DESTROY_BOSS): // Destroy Boss
            destroySprite(bossSprites_);
            break;
        case static_cast<int>(Network::PacketType::DESTROY_BULLET): // Destroy Bullet
            destroySprite(fastBulletSprites_);
            break;
        default:
            break;
    }
    resetValues();
}

void GameScene::updateUI() {
    health_ = static_cast<int>(packet_data_.new_x);
    score_ = static_cast<int>(packet_data_.new_y);
}

void GameScene::setTextProperties(sf::Text& text, const sf::Font& font, unsigned int size, const sf::Color& color, float x, float y) {
    text.setFont(font);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(x, y);
}

void GameScene::setUI(sf::RenderWindow& window)
{
    if (!font_.loadFromFile("../assets/font.otf"))
        std::cerr << "[ERROR] loading font file" << std::endl;

    sf::Text scoreText;
    setTextProperties(scoreText, font_, 24, sf::Color::White, 10, 10);
    uiSprites[0] = scoreText;

    sf::Text timerText;
    setTextProperties(timerText, font_, 48, sf::Color::Green, window.getSize().x / 2 - 50, 10);
    uiSprites[1] = timerText;

    sf::Text healthText;
    setTextProperties(healthText, font_, 24, sf::Color::Red, 10, window.getSize().y - 30);
    uiSprites[2] = healthText;

    sf::Text latencyText;
    setTextProperties(latencyText, font_, 16, sf::Color::White, window.getSize().x - 120, 10);
    uiSprites[3] = latencyText;

    sf::Text packetLossText;
    setTextProperties(packetLossText, font_, 16, sf::Color::White, window.getSize().x - 120, 30);
    uiSprites[4] = packetLossText;
}


void GameScene::setStats(sf::RenderWindow& window) {
    uiSprites[0].setString("Health: " + std::to_string(health_));
    uiSprites[1].setString("Score: " + std::to_string(score_));

    sf::Time elapsed = game_clock_.getElapsedTime();
    int totalSeconds = static_cast<int>(elapsed.asSeconds());
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    uiSprites[2].setString("Time: " + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds));

    static sf::Clock latencyClock;
    if (latencyClock.getElapsedTime().asSeconds() >= 3) {
        uiSprites[3].setString("Latency: " + std::to_string(static_cast<int>(lag_meter_.ping)) + "ms");
        latencyClock.restart();
    }
    uiSprites[4].setString("Packet Loss: " + std::to_string(static_cast<int>(lag_meter_.packets_lost_)) + "%");
}

void GameScene::updatePositions() {
    for (auto& [id, spriteElement] : bulletSprites_) {
        spriteElement.sprite.move(spriteElement.vx, spriteElement.vy);
    }
    for (auto& [id, spriteElement] : enemySprites_) {
        spriteElement.sprite.move(spriteElement.vx, spriteElement.vy);
    }
    for (auto& [id, spriteElement] : bossSprites_) {
        spriteElement.sprite.move(spriteElement.vx, spriteElement.vy);
    }
    for (auto& [id, spriteElement] : fastBulletSprites_) {
        spriteElement.sprite.move(spriteElement.vx, spriteElement.vy);
    }
}

void GameScene::drawSprites(sf::RenderWindow& window) {
    for (auto& [id, spriteElement] : sceneSprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& [id, spriteElement] : playerSprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& [id, spriteElement] : enemySprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& [id, spriteElement] : bulletSprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& [id, spriteElement] : fastBulletSprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& [id, spriteElement] : bossSprites_) {
        window.draw(spriteElement.sprite);
    }
    for (auto& [id, text] : uiSprites) {
        window.draw(text);
    }
}

void GameScene::handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                std::cout << playerSprites_.size() << std::endl;
                playerSprites_.erase(packet_data_.server_id);
                destroySprite(playerSprites_);
                if (checkGameEnd()) {
                    newAction_ = Network::PacketType::GAME_END;
                }
                window.close();
                break;
            case sf::Event::KeyPressed:
                handleKeyPress(event.key.code, window);
                break;
            default:
                break;
        }
    }
}

void GameScene::handleKeyPress(sf::Keyboard::Key key, sf::RenderWindow& window)
{
    if (!playerAliveStatus_[packet_data_.server_id]) {
        return;
    }

    switch (key) {
        case sf::Keyboard::Right:
            newAction_ = Network::PacketType::PLAYER_RIGHT;
            break;

        case sf::Keyboard::Left:
            newAction_ = Network::PacketType::PLAYER_LEFT;
            break;

        case sf::Keyboard::Up:
            newAction_ = Network::PacketType::PLAYER_UP;
            break;

        case sf::Keyboard::Down:
            newAction_ = Network::PacketType::PLAYER_DOWN;
            break;

        case sf::Keyboard::Space:
                newAction_ = Network::PacketType::PLAYER_SHOOT;
        break;
        case sf::Keyboard::Escape:
            window.close();
            newAction_ = Network::PacketType::GAME_END;
            break;
        case sf::Keyboard::Num1:
            break;
        case sf::Keyboard::Num2:
            break;
        default:
            break;
    }
}

void GameScene::setPacketData(PacketData& packetData) {
    packet_data_ = packetData;
}

std::optional<Network::PacketType> GameScene::sendOverPackets() {
    auto action = newAction_;
    newAction_.reset();
    return action;
}

void GameScene::setLagMeter(LagMeter& lm) {
    lag_meter_ = lm;
}

bool GameScene::checkGameEnd() {
    if (playerSprites_.empty()) {
        return true;
    }
    return false;
}

void GameScene::cleanup(sf::RenderWindow& window) {
    window.clear();
    playerSprites_.clear();
    enemySprites_.clear();
    bulletSprites_.clear();
    sceneSprites_.clear();
    bossSprites_.clear();
    uiSprites.clear();
}