#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Registry.hpp"

class Player {
public:
    Player(Registry& registry, float x, float y);
    ~Player();

    void move(float x, float y);
    Registry::Entity getEntity() const;

    const Registry& getRegistry() const;
    void setRegistry(const Registry& newRegistry);

    std::pair<float, float> getVelocity() const;
    void setVelocity(float x, float y);

private:
    Registry& registry;
    Registry::Entity entity;
    int health;
};

#endif // PLAYER_HPP