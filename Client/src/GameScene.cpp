#include "GameScene.hpp"
#include "Client.hpp"
#include "RTYPEPlayer.hpp"
#include "RTYPEEnemy.hpp"
#include "RTYPEBoss.hpp"
#include "PositionSystem.hpp"
#include "DrawSystem.hpp"
#include "ControlSystem.hpp"
#include "PacketType.hpp"
#include <iostream>
#include "Initialization.hpp"

GameScene::GameScene(sf::RenderWindow& window, Client& client)
    : Scene(window, client), window_(window), client_(client)
{
    std::cout << "[DEBUG] GameScene constructor called" << std::endl;

    initBackground();
    initializeECS(registry_, true, &window);
    window.setKeyRepeatEnabled(false);
}

GameScene::~GameScene() {}

void GameScene::initBackground() {
    std::cout << "[DEBUG] Initializing game sprites" << std::endl;
    gameTexts_.clear();

    if (!backgroundTexture.loadFromFile("../assets/background.png")) {
        std::cerr << "[ERROR] Failed to load background texture" << std::endl;
    }
    backgroundSprite.setTexture(backgroundTexture);
    std::cout << "[DEBUG] Game sprites initialized" << std::endl;
}

void GameScene::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::KeyPressed) {
            handleKeyPress(event.key.code);
        } else if (event.type == sf::Event::KeyReleased) {
            handleKeyUnpress(event.key.code);
        }
    }
}

void GameScene::handleKeyPress(sf::Keyboard::Key key) {
    const auto& entity = players[client.server_id]->getEntity();
    auto ctrl = registry_.get_components<Controllable>()[entity];

    switch (key) {
        case sf::Keyboard::Right:
            ctrl->moveRight = true;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_RIGHT));
            break;
        case sf::Keyboard::Left:
            ctrl->moveLeft = true;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_LEFT));
            break;
        case sf::Keyboard::Up:
            ctrl->moveUp = true;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_UP));
            break;
        case sf::Keyboard::Down:
            ctrl->moveDown = true;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_DOWN));
            break;
        case sf::Keyboard::Space:
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_SHOOT));
            break;
        case sf::Keyboard::B:
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_BLAST));
            break;
        default:
            break;
    }
}

void GameScene::handleKeyUnpress(sf::Keyboard::Key key) {
    const auto& player = players[client.server_id];
    const auto &entity = player->getEntity();
    auto& ctrl = registry_.get_components<Controllable>()[entity];

    switch (key) {
        case sf::Keyboard::Right:
            ctrl->moveRight = false;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_STOP_R));
            break;
        case sf::Keyboard::Left:
            ctrl->moveLeft = false;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_STOP_L));
            break;
        case sf::Keyboard::Up:
            ctrl->moveUp = false;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_STOP_U));
            break;
        case sf::Keyboard::Down:
            ctrl->moveDown = false;
            client.send_queue_.push(client.createPacket(Network::PacketType::PLAYER_STOP_D));
            break;
        default:
            break;
    }
}


void GameScene::handleServerActions() {
    const auto& player = players[client.server_id];

    switch (client.action) {
        case static_cast<int>(Network::PacketType::PLAYER_STOP_U):
            std::cout << client.server_id << client.new_x << client.new_y << std::endl;
            player->setPosition(client.new_x, client.new_y);
            break;
        case static_cast<int>(Network::PacketType::PLAYER_STOP_D):
            player->setPosition(client.new_x, client.new_y);
            break;
        case static_cast<int>(Network::PacketType::PLAYER_STOP_L):
            player->setPosition(client.new_x, client.new_y);
            break;
        case static_cast<int>(Network::PacketType::PLAYER_STOP_R):
            player->setPosition(client.new_x, client.new_y);
            break;
        case static_cast<int>(Network::PacketType::PLAYER_SHOOT):
            bullets.emplace(bulletId, std::make_unique<Bullet>(registry_, client.new_x, client.new_y, 4.f, 1.0f, 0.0f));
            bulletId++;
            break;
        case static_cast<int>(Network::PacketType::PLAYER_BLAST):
            break;
        case static_cast<int>(Network::PacketType::PLAYER_CREATE):
            std::cout << "Player " << client.server_id << " spawned at: " << client.new_x << ", " << client.new_y << std::endl;
            if (players.find(client.server_id) == players.end()) {
                players.emplace(client.server_id, std::make_unique<Player>(registry_, client.new_x, client.new_y));
            }
            break;
        case static_cast<int>(Network::PacketType::ENEMY_CREATE):
            enemies.emplace(enemyId, std::make_unique<Enemy>(registry_, client.new_x, client.new_y));
            enemyId++;
            break;
        case static_cast<int>(Network::PacketType::BOSS_CREATE):
        break;
        default:
            break;
    }
    // Update player position based on server response
    client.resetValues();
}

void GameScene::update() {
    handleServerActions();
    registry_.run_systems();
}

void GameScene::render() {
    window.clear();
    window.draw(backgroundSprite);

    drawSystem(registry_, window_, registry_.get_components<Position>(), registry_.get_components<Drawable>());
    renderOverlay();
    window.display();
}