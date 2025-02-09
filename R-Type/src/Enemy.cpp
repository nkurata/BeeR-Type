
#include "Enemy.hpp"
#include "Position.hpp"
#include "Drawable.hpp"
#include "Collidable.hpp"
#include <iostream>

Enemy::Enemy(Registry registry, float x, float y) : registry(registry) {
    entity = this->registry.spawn_entity();
    this->registry.add_component<Position>(entity, {x, y});
}

Registry::Entity Enemy::getEntity() const {
    return entity;
}

void Enemy::move(float x, float y) {
    if (registry.has_component<Position>(entity)) {
        auto& pos = registry.get_components<Position>()[entity];
        pos->x += x;
        pos->y += y;
    } else {
        std::cerr << "Error: Enemy entity does not have a Position component." << std::endl;
    }
}

Enemy::~Enemy() {
    if (registry.entity_exists(entity))
        registry.kill_entity(entity);
}

const Registry& Enemy::getRegistry() const {
    return registry;
}

