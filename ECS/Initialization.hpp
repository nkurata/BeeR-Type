#ifndef INITIALIZATION_HPP
    #define INITIALIZATION_HPP

#include "Registry.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include "Projectile.hpp"
#include "Drawable.hpp"
#include "PositionSystem.hpp"
#include "ProjectileSystem.hpp"
#include "CollisionSystem.hpp"
#include "DrawSystem.hpp"
#include "ControlSystem.hpp"

inline void initializeECS(Registry& registry, bool isClient, sf::RenderWindow* window = nullptr) {
    registry.register_component<Position>();
    registry.register_component<Velocity>();
    registry.register_component<Controllable>();
    registry.register_component<Collidable>();
    registry.register_component<Projectile>();

    if (isClient) {
        registry.register_component<Drawable>();
        registry.add_system<Position, Drawable>([window](Registry& registry, sparse_array<Position>& positions, sparse_array<Drawable>& drawables) {
            drawSystem(registry, *window, positions, drawables);
        });
    }

    registry.add_system<Position, Velocity>(positionSystem);
    registry.add_system<Position, Velocity, Projectile>(projectileSystem);
    registry.add_system<Position, Collidable, Controllable, Projectile>(collisionSystem);

    if (isClient) {
        registry.add_system<Velocity, Controllable>(controlSystem);
    }
}

#endif // INITIALIZATION_HPP
