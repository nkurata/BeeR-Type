#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Registry.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Controllable.hpp"
#include "RTYPEBullet.hpp"
#include "SFML/Graphics.hpp"
#include <vector>

class Player {
public:
    Player(Registry& registry, float x, float y) : registry(registry) {
        entity = registry.spawn_entity();
        registry.emplace_component<Position>(entity, x, y);
        registry.emplace_component<Velocity>(entity, 0.0f, 0.0f);
        registry.emplace_component<Drawable>(entity, "../assets/player.png");
        registry.emplace_component<Controllable>(entity);
    }

    ~Player() {}

    Registry::Entity getEntity() const {
        return entity;
    }

    void setVelocity(float vx, float vy) {
        auto& vel = registry.get_components<Velocity>()[entity];
        if (!vel) {
            std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
        } else {
            vel->vx = vx;
            vel->vy = vy;
        }
    }

    void setPosition (float x, float y) {
        auto& pos = registry.get_components<Position>()[entity];
        if (!pos) {
            std::cerr << "Error: Player entity does not have a Position component." << std::endl;
        } else {
            pos->x = x;
            pos->y = y;
        }
    }

    std::pair<float, float> getVelocity() const {
        const auto& velocity = registry.get_components<Velocity>()[entity];
        if (!velocity) {
            std::cerr << "Error: Player entity does not have a Velocity component." << std::endl;
            return {0.0f, 0.0f};
        } else {
            return {velocity->vx, velocity->vy};
        }
    }

private:
    Registry& registry;
    Registry::Entity entity;
};

#endif // PLAYER_HPP