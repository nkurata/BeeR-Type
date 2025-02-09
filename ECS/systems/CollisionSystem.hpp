#ifndef COLLISIONSYSTEM_H
    #define COLLISIONSYSTEM_H


#include "Registry.hpp"
#include "Position.hpp"
#include "Collidable.hpp"
#include "Projectile.hpp"
#include "Controllable.hpp"
#include <vector>
#include <iostream>

inline bool checkCollision(const Position& pos1, const Hitbox& hitbox1, const Position& pos2, const Hitbox& hitbox2) {
    return (pos1.x < pos2.x + hitbox2.width &&
            pos1.x + hitbox1.width > pos2.x &&
            pos1.y < pos2.y + hitbox2.height &&
            pos1.y + hitbox1.height > pos2.y);
}

inline std::vector<std::pair<size_t, size_t>> collisionSystem(Registry& registry, sparse_array<Position>& positions, sparse_array<Collidable>& collidables, sparse_array<Controllable>& controllables, sparse_array<Projectile>& projectiles) {
    std::vector<std::pair<size_t, size_t>> collisions;
    for (size_t i = 0; i < positions.size() && i < collidables.size(); ++i) {
        auto& pos = positions[i];
        auto& collidable = collidables[i];
        if (pos && collidable && collidable->is_collidable) {
            for (size_t j = i + 1; j < positions.size() && j < collidables.size(); ++j) {
                auto& otherPos = positions[j];
                auto& otherCollidable = collidables[j];
                if (otherPos && otherCollidable && otherCollidable->is_collidable) {
                    if (checkCollision(*pos, collidable->hitbox, *otherPos, otherCollidable->hitbox)) {
                        std::cout << "Collision detected between entity " << i << " and entity " << j << std::endl;
                        collisions.emplace_back(i, j); // Record collision
                        if (controllables[i] || controllables[j]) {
                            registry.kill_entity(controllables[i] ? i : j);
                        }
                        if (projectiles[i] || projectiles[j]) {
                            std::cout << "Projectile collision: killing entities " << i << " and " << j << std::endl;
                            registry.kill_entity(projectiles[i] ? j : i);
                            registry.kill_entity(projectiles[i] ? i : j);
                        } else {
                            if (!controllables[i] && !controllables[j]) {
                                registry.kill_entity(i);
                                registry.kill_entity(j);
                            }
                        }
                    }
                }
            }
        }
    }
    return collisions; // Return list of collisions
}

#endif // COLLISIONSYSTEM_H