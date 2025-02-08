#ifndef BOSS_HPP
#define BOSS_HPP

#include "Registry.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "SFML/Graphics.hpp"


class Boss {
public:
    Boss(Registry& registry, float x, float y) : registry(registry) {
        entity = registry.spawn_entity();
        registry.add_component<Position>(entity, {x, y});
        registry.add_component<Velocity>(entity, {0.0f, 0.0f});
        registry.add_component<Drawable>(entity, {"../assets/boss.png"});
    }

    ~Boss() {
        registry.kill_entity(entity);
    }

    Registry::Entity getEntity() const {
        return entity;
    }

private:
    Registry& registry;
    Registry::Entity entity;
};

#endif // Boss_HPP