#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Registry.hpp"

class Player {
public:
    Player(Registry& registry, float x, float y);
    ~Player();

    void move(float vx, float vy);
    void stop();

    Registry::Entity getEntity() const;

    std::pair<float, float> getVelocity() const;

    int getHealth() const;

private:
    Registry& registry;
    Registry::Entity entity;
    int health;
};

#endif // PLAYER_HPP