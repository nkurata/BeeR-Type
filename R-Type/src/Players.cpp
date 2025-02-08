/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** Player
*/

#include "Players.hpp"

#include <chrono>

#include "Position.hpp"
#include "Velocity.hpp"
#include "Collidable.hpp"
#include <iostream>

Player::Player(Registry& registry, float x, float y) : registry(registry) {
    entity = registry.spawn_entity();
    registry.add_component<Position>(entity, {x, y});
    registry.add_component<Velocity>(entity, {0.0f, 0.0f});
    registry.add_component<Collidable>(entity, {true});
    health = 100;
}

Player::~Player() { }

void Player::move(float vx, float vy) {
    static auto up = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - up).count();
    if (elapsed >= 16) {
        auto& pos = registry.get_components<Position>()[entity];
        auto& vel = registry.get_components<Velocity>()[entity];
        if (!pos) {
            std::cerr << "Error: Player entity does not have a Position component." << std::endl;
        } else if (!vel) {
            std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
        } else {
            vel->vx = vx;
            vel->vy = vy;
            pos->x += vel->vx;
            pos->y += vel->vy;
        }
        up = now;
    }
}

std::pair<float, float> Player::getVelocity() const {
    const auto& velocity = registry.get_components<Velocity>()[entity];
    if (!velocity) {
        std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
        return {0.0f, 0.0f};
    } else {
        return {velocity->vx, velocity->vy};
    }
}

std::pair<float, float> Player::getPosition() const {
    const auto& position = registry.get_components<Position>()[entity];
    if (!position) {
        std::cerr << "Error: Player entity does not have a Position component." << std::endl;
        return {0.0f, 0.0f};
    } else {
        return {position->x, position->y};
    }
}

Registry::Entity Player::getEntity() const {
    return entity;
}

int Player::getHealth() const {
    return health;
}

void Player::printPosition() {
    const auto& pos = registry.get_components<Position>()[entity];
    if (!pos) {
        std::cerr << "Error: Player entity does not have a Position component." << std::endl;
    } else {
        std::cout << "Player position: " << pos->x << ", " << pos->y << std::endl;
    }
}

std::string Player::playerShoot() {
    const auto &playerPosition = registry.get_components<Position>()[entity];
    if (!playerPosition.has_value()) {
        throw std::runtime_error("Position component not found for entity");
    }
    const float x = playerPosition->x;
    const float y = playerPosition->y;
    std::cout << "[DEBUG] Player shot from " << x << ", " << y << std::endl;
    bullets_.emplace_back(new Bullet(registry, x, y, 4.f));
    return std::to_string(bullets_.back()->getEntity()) + ";" + std::to_string(x) + ";" + std::to_string(y);
}

std::string Player::playerBlast () {
    auto &playerPosition = registry.get_components<Position>()[entity];
    if (!playerPosition.has_value()) {
        throw std::runtime_error("Position component not found for entity");
    }
    bullets_.emplace_back(new Bullet(registry, playerPosition->x, playerPosition->y, 0.5f));
    health -= 10;
    return std::to_string(bullets_.back()->getEntity()) + ";" + std::to_string(playerPosition->x) + ";" + std::to_string(playerPosition->y);
}

std::vector<std::unique_ptr<Bullet>>& Player::getBullets() {
    return bullets_;
}