
#include "Bullet.hpp"
#include "Position.hpp"
#include "Drawable.hpp"
#include "Projectile.hpp"
#include "Collidable.hpp"
#include <iostream>

Bullet::Bullet(Registry registry, int id, float x, float y, float speed) : registry(registry) {
    this->entity = this->registry.spawn_entity();
    this->registry.add_component<Position>(this->entity, {x, y});
}

Bullet::~Bullet() {
    if (registry.entity_exists(entity))
        registry.kill_entity(entity);
}


void Bullet::move(float x, float y) {
    if (registry.has_component<Position>(entity)) {
        auto& pos = registry.get_components<Position>()[entity];
        pos->x += x;
        pos->y += y;
    } else {
        std::cerr << "Error: Bullet entity does not have a Position component." << std::endl;
    }
}

Registry::Entity Bullet::getEntity() const {
    return entity;
}

const Registry& Bullet::getRegistry() const {
    return registry;
}
