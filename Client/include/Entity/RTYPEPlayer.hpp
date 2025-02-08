#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Registry.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "RTYPEBullet.hpp"
#include "SFML/Graphics.hpp"
#include <vector>

class Player {
public:
    Player(Registry& registry, float x, float y) : registry(registry) {
        entity = registry.spawn_entity();
        registry.add_component<Position>(entity, {x, y});
        registry.add_component<Velocity>(entity, {0.0f, 0.0f});  
        registry.add_component<Drawable>(entity, {"../assets/player.png"});
    }

    ~Player() {
        registry.kill_entity(entity);
    }

    Registry::Entity getEntity() const {
        return entity;
    }

    void changeVelocity(float vx, float vy) {
        auto& velocityOpt = registry.get_components<Velocity>()[entity];
        if (!velocityOpt.has_value()) {
            throw std::runtime_error("Velocity component not found for entity");
        }
        auto& velocityComponent = velocityOpt.value();
        velocityComponent.vx = vx;
        velocityComponent.vy = vy;
    }

    void playerShoot() {
        auto &playerPosition = registry.get_components<Position>()[entity];
        if (!playerPosition.has_value()) {
            throw std::runtime_error("Position component not found for entity");
        }
        bullets_.emplace_back(new Bullet(registry, playerPosition->x, playerPosition->y, 0.5f));
    }

    void playerBlast () {
        auto &playerPosition = registry.get_components<Position>()[entity];
        if (!playerPosition.has_value()) {
            throw std::runtime_error("Position component not found for entity");
        }
        bullets_.emplace_back(new Bullet(registry, playerPosition->x, playerPosition->y, 0.5f));
        bullets_.emplace_back(new Bullet(registry, playerPosition->x, playerPosition->y, 0.5f));
        bullets_.emplace_back(new Bullet(registry, playerPosition->x, playerPosition->y, 0.5f));
    }

    const std::vector<Bullet*>& getBullets() const {
        return bullets_;
    }

    void moveUp () {
        changeVelocity(0.0f, -0.1f);
    }

    void moveDown () {
        changeVelocity(0.0f, 0.1f);
    }

    void moveLeft () {
        changeVelocity(-0.1f, 0.0f);
    }

    void moveRight () {
        changeVelocity(0.1f, 0.0f);
    }

    void stop() {
        changeVelocity(0.0f, 0.0f);
    }

private:
    Registry& registry;
    Registry::Entity entity;
    std::vector<Bullet*> bullets_;
};

#endif // PLAYER_HPP