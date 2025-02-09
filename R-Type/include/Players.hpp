#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Registry.hpp"

class Player {
public:
    Player();
    Player(Registry registry, float x, float y);
    ~Player();

    void move(float x, float y);
    void updateHp(float amount);
    void updateScore(float amount);
    float getHp() const;
    float getScore() const;

    Registry::Entity getEntity() const;
    const Registry& getRegistry() const;

private:
    Registry registry;
    Registry::Entity entity;
    float hp = 0;
    float score = 0;
};

#endif // PLAYER_HPP