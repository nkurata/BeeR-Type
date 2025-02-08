#ifndef BULLET_HPP
#define BULLET_HPP

#include "Registry.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Drawable.hpp"
#include "SFML/Graphics.hpp"

class Bullet {
public:
    Bullet(Registry& registry, float x, float y, float speed) : registry(registry) {
        entity = registry.spawn_entity();
        registry.add_component<Position>(entity, {x, y});
        registry.add_component<Projectile>(entity, {speed});
        registry.add_component<Drawable>(entity, {"../assets/bullet.png"});
    }

    Registry::Entity getEntity() const {
        return entity;
    }

    void setVelocity(float vx, float vy) {
        auto& velocity = registry.get_components<Velocity>()[entity];
        if (!velocity) {
            throw std::runtime_error("Velocity component not found for entity");
        }
        velocity->vx = vx;
        velocity->vy = vy;
    }

private:
    Registry& registry;
    Registry::Entity entity;
};

#endif // BULLET_HPP