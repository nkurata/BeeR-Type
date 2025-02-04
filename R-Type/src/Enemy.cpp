/*
** EPITECH PROJECT, 2025
** R-Type [WSL: Ubuntu]
** File description:
** Enemy
*/

#include "Enemy.hpp"
#include "Position.hpp"
#include "Drawable.hpp"
#include "Collidable.hpp"
#include <iostream>

Enemy::Enemy(Registry &registry, float x, float y) : registry(registry) {
    entity = registry.spawn_entity();
    registry.add_component<Position>(entity, {x, y});
    registry.add_component<Drawable>(entity, {});
    registry.add_component<Collidable>(entity, {true});
}

Enemy::~Enemy() {
    registry.kill_entity(entity);
}

Registry::Entity Enemy::getEntity() const {
    return entity;
}

void Enemy::move(float x, float y) {
    auto& pos = registry.get_components<Position>()[entity];
    if (!pos) {
        std::cerr << "Error: Enemy entity does not have a Position component." << std::endl;
    } else {
        pos->y += y;
        pos->x += x;
    }
}

const Registry& Enemy::getRegistry() const {
    return registry;
}

void Enemy::setRegistry(const Registry& newRegistry) {
    registry = newRegistry;
}
