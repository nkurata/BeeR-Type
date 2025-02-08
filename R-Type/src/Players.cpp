/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** Player
*/

#include "Players.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include <iostream>

Player::Player(Registry& registry, float x, float y) : registry(registry) {
    entity = registry.spawn_entity();
    registry.add_component<Position>(entity, {x, y});
    registry.add_component<Velocity>(entity, {0.0f, 0.0f});
    registry.add_component<Controllable>(entity, {false, false, false, false});
    registry.add_component<Collidable>(entity, {true});
    health = 100;
}

Player::~Player() {
	registry.kill_entity(entity);
}

void Player::move(float vx, float vy) {
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
}

void Player::stop() {
    auto& vel = registry.get_components<Velocity>()[entity];
    if (!vel) {
        std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
    } else {
        vel->vx = 0.0f;
        vel->vy = 0.0f;
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

Registry::Entity Player::getEntity() const {
    return entity;
}

int Player::getHealth() const {
    return health;
}