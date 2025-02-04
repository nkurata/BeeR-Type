/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** Bullet
*/

#include "Bullet.hpp"
#include "Position.hpp"
#include "Drawable.hpp"
#include "Projectile.hpp"
#include <iostream>

Bullet::Bullet(Registry& registry, float x, float y, float speed) : registry(registry) {
    entity = registry.spawn_entity();
    registry.add_component<Position>(entity, {x, y});
    registry.add_component<Projectile>(entity, {speed});
    registry.add_component<Drawable>(entity, {sf::RectangleShape(sf::Vector2f(5.0f, 5.0f))});
}

Bullet::~Bullet() {
    registry.kill_entity(entity);
}


void Bullet::move(float x, float y) {
    auto& pos = registry.get_components<Position>()[entity];
    if (!pos) {
        std::cerr << "Error: Bullet entity does not have a Position component." << std::endl;
    } else {
        pos->x += x;
        pos->y += y;
    }
}

Registry::Entity Bullet::getEntity() const {
    return entity;
}

const Registry& Bullet::getRegistry() const {
    return registry;
}

void Bullet::setRegistry(const Registry& newRegistry) {
    registry = newRegistry;
}
