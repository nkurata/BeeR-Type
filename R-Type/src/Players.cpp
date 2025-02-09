
#include "Players.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Drawable.hpp"
#include "Controllable.hpp"
#include "Collidable.hpp"
#include <iostream>
#include <utility>

Player::Player() : registry(), entity(0) {}

Player::Player(Registry registry,  float x,  float y) : registry(registry) {
    entity = this->registry.spawn_entity();
    this->registry.add_component<Position>(entity, {x, y});
}

Player::~Player() {
	if (registry.entity_exists(entity))
		registry.kill_entity(entity);
}

void Player::move(const float x, const float y) {
  	if (registry.has_component<Position>(entity)) {
    	auto& pos = registry.get_components<Position>()[entity];
   		pos->x += x;
    	pos->y += y;
    } else {
		std::cerr << "Error: Player entity does not have a Position component." << std::endl;
    }
}

Registry::Entity Player::getEntity() const {
    return entity;
}

const Registry& Player::getRegistry() const {
    return registry;
}

void Player::updateHp(float amount) {
    hp += amount;
}

void Player::updateScore(float amount) {
    score += amount;
}

float Player::getHp() const {
    return hp;
}

float Player::getScore() const {
    return score;
}