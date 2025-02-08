#include "GameScene.hpp"
#include "Client.hpp"
#include "RTYPEPlayer.hpp"
#include "RTYPEEnemy.hpp"
#include "RTYPEBoss.hpp"
#include "RTYPEBullet.hpp"
#include "PacketType.hpp"
#include <iostream>

GameScene::GameScene(sf::RenderWindow& window, Client& client)
    : Scene(window, client), window_(window), client_(client)
{
    std::cout << "[DEBUG] GameScene constructor called" << std::endl;

    // Register systems
    initBackground();
    registry_.register_component<Position>();
    registry_.register_component<Velocity>();
    registry_.register_component<Drawable>();
    registry_.add_system<Position, Velocity>(positionSystem);
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

void GameScene::createSprite() {
    switch (client.action) {
        case static_cast<int>(Network::PacketType::PLAYER_CREATE):
            std::cout << "Player " << client.server_id << " is being created" << std::endl;
            players_.emplace(client.server_id, new Player(registry_, client.new_x, client.new_y));
            // client.send(client.createPacket(Network::PacketType::PLAYER_CREATE));
            break;
        case static_cast<int>(Network::PacketType::ENEMY_CREATE):
            std::cout << "Enemy " << client.server_id << " is being created" << std::endl;
            enemies_.emplace(client.server_id, new Enemy(registry_, client.new_x, client.new_y));
            break;
        case static_cast<int>(Network::PacketType::BOSS_CREATE):
            std::cout << "Boss " << client.server_id << " is being created" << std::endl;
            bosses_.emplace(client.server_id, new Boss(registry_, client.new_x, client.new_y));
            break;
        // case static_cast<int>(Network::PacketType::BULLET_CREATE):
        //     std::cout << "Bullet " << client.server_id << " is being created" << std::endl;
        //     players_[client.server_id].playerShoot();
        //     break;
        default:
            break;
    }
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

        case sf::Mouse::Left:
            client.send(client.createPacket(Network::PacketType::PLAYER_SHOOT));
            break;
            
        case sf::Mouse::Right:
            client.send(client.createPacket(Network::PacketType::PLAYER_BLAST));
            break;

        case sf::Keyboard::Escape:
            client.send(client.createPacket(Network::PacketType::GAME_END));
            break;

        default:
            break;
    }
}

void GameScene::handleKeyUnpress(sf::Keyboard::Key key)
{
    switch (key) {
        case sf::Keyboard::Right:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP));
            break;

        case sf::Keyboard::Left:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP));
            break;

        case sf::Keyboard::Up:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP));
            break;

        case sf::Keyboard::Down:
            client.send(client.createPacket(Network::PacketType::PLAYER_STOP));
            break;

        default:
            break;
    }
}

void GameScene::handleServerActions() {
    switch (client.action) {
        case static_cast<int>(Network::PacketType::PLAYER_UP):
            players_[client.server_id]->moveUp();
            break;
        case static_cast<int>(Network::PacketType::PLAYER_DOWN):
            players_[client.server_id]->moveDown();
            break;
        case static_cast<int>(Network::PacketType::PLAYER_LEFT):
            players_[client.server_id]->moveLeft();
            break;
        case static_cast<int>(Network::PacketType::PLAYER_RIGHT):
            players_[client.server_id]->moveRight();
            break;
        case static_cast<int>(Network::PacketType::PLAYER_STOP):
            players_[client.server_id]->stop();
            break;
        case static_cast<int>(Network::PacketType::PLAYER_SHOOT):
            players_[client.server_id]->playerShoot();
            break;
        case static_cast<int>(Network::PacketType::PLAYER_BLAST):
            players_[client.server_id]->playerBlast();
            break;
        case static_cast<int>(Network::PacketType::GAME_END):
            client.switchScene(SceneType::Lobby);
            break;
        default:
            break;
    }
}

void GameScene::update() {
    createSprite();
    handleServerActions();
    registry_.run_systems();
}

void GameScene::render() {
    window.clear();
    window.draw(backgroundSprite);

    // Draw players
    for (const auto& [id, player] : players_) {
        auto& drawableOpt = registry_.get_components<Drawable>()[player->getEntity()];
        if (drawableOpt.has_value()) {
            auto& drawable = drawableOpt.value();
            window.draw(drawable.sprite);
        }
    }

    // // Draw enemies
    // for (const auto& [id, enemy] : enemies_) {
    //     auto& drawableOpt = registry_.get_components<Drawable>()[enemy->getEntity()];
    //     if (drawableOpt.has_value()) {
    //         auto& drawable = drawableOpt.value();
    //         window.draw(drawable.sprite);
    //     }
    // }

    // // Draw bosses
    // for (const auto& [id, boss] : bosses_) {
    //     auto& drawableOpt = registry_.get_components<Drawable>()[boss->getEntity()];
    //     if (drawableOpt.has_value()) {
    //         auto& drawable = drawableOpt.value();
    //         window.draw(drawable.sprite);
    //     }
    // }

    // // Draw bullets
    // for (const auto& [id, player] : players_) {
    //     for (const auto& bullet : player->getBullets()) {
    //         auto& drawableOpt = registry_.get_components<Drawable>()[bullet->getEntity()];
    //         if (drawableOpt.has_value()) {
    //             auto& drawable = drawableOpt.value();
    //             window.draw(drawable.sprite);
    //         }
    //     }
    // }

    window.display();
}
