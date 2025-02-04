/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** Boss
*/

#include "Boss.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Collidable.hpp"
#include <iostream>

Boss::Boss(Registry& registry, float x, float y) : registry(registry) {
    entity = registry.spawn_entity();
    registry.add_component<Position>(entity, {x, y});
    registry.add_component<Velocity>(entity, {0.0f, 0.0f});
    registry.add_component<Drawable>(entity, {sf::RectangleShape(sf::Vector2f(100.0f, 100.0f))});
    registry.add_component<Collidable>(entity, {true});
}

Boss::~Boss() {
    registry.kill_entity(entity);
}

void Boss::move(float x, float y) {
    auto& pos = registry.get_components<Position>()[entity];
    if (!pos) {
        std::cerr << "Error: Boss entity does not have a Position component." << std::endl;
    } else {
        pos->x += x;
        pos->y += y;
    }
}

Registry::Entity Boss::getEntity() const {
    return entity;
}

const Registry& Boss::getRegistry() const {
    return registry;
}

void Boss::setRegistry(const Registry& newRegistry) {
    registry = newRegistry;
}
