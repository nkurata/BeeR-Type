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

    void renderPlayer(sf::RenderWindow& window) {
        auto& positionOpt = registry.get_components<Position>()[entity];
        if (!positionOpt.has_value()) {
            throw std::runtime_error("Position component not found for entity");
        }
        auto& drawableOpt = registry.get_components<Drawable>()[entity];
        if (!drawableOpt.has_value()) {
            throw std::runtime_error("Drawable component not found for entity");
        }
        auto& positionComponent = positionOpt.value();
        auto& drawableComponent = drawableOpt.value();
        drawableComponent.sprite.setPosition(positionComponent.x, positionComponent.y);
        window.draw(drawableComponent.sprite);
    }

    const std::vector<Bullet*>& getBullets() const {
        return bullets_;
    }

private:
    Registry& registry;
    Registry::Entity entity;
    std::vector<Bullet*> bullets_;
};

#endif // PLAYER_HPP