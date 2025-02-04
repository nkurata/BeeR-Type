/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** Player
*/

#include "Player.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include <iostream>

Player::Player(Registry& registry, float x, float y) : registry(registry) {
    entity = this->registry.spawn_entity();
    this->registry.add_component<Position>(entity, {x, y});
    this->registry.add_component<Velocity>(entity, {0.0f, 0.0f});
    this->registry.add_component<Drawable>(entity, {});
    this->registry.add_component<Controllable>(entity, {});
    this->registry.add_component<Collidable>(entity, {true});
    health = 100;
}

Player::~Player() {
	if (registry.entity_exists(entity))
		registry.kill_entity(entity);
}

void Player::move(float x, float y) {
    if (registry.has_component<Position>(entity)) {
        auto& pos = registry.get_components<Position>()[entity];
        pos->x += x;
        pos->y += y;
    } else {
        std::cerr << "Error: Player entity does not have a Position component." << std::endl;
    }
}

std::pair<float, float> Player::getVelocity() const {
    if (registry.has_component<Velocity>(entity)) {
        const auto& velocity = registry.get_components<Velocity>()[entity];
        return {velocity->vx, velocity->vy};
    } else {
        std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
        return {0.0f, 0.0f};
    }
}

Registry::Entity Player::getEntity() const {
    return entity;
}

const Registry& Player::getRegistry() const {
    return registry;
}

void Player::setRegistry(const Registry& newRegistry) {
    registry = newRegistry;
}

void Player::setVelocity(float x, float y) {
    if (registry.has_component<Velocity>(entity)) {
        auto& velocity = registry.get_components<Velocity>()[entity];
        velocity->vx = x;
        velocity->vy = y;
    } else {
        std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
    }
}