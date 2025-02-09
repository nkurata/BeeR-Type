#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Registry.hpp"
#include "Bullet.hpp"
#include <vector>


class Player {
public:
    Player(Registry& registry, float x, float y);
    ~Player();

    void move(float vx, float vy);
    Registry::Entity getEntity() const;
    std::pair<float, float> getVelocity() const;
    std::pair<float, float> getPosition() const;
    int getHealth() const;

    void printPosition();

private:
    Registry& registry;
    Registry::Entity entity;
    int health;
};

#endif // PLAYER_HPP