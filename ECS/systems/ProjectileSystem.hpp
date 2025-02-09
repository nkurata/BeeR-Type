/*
** EPITECH PROJECT, 2024
** R-Type ECS
** File description:
** ProjectileSystem
*/

#ifndef PROJECTILESYSTEM_H
    #define PROJECTILESYSTEM_H

#include "Registry.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Velocity.hpp"
#include <iostream>

inline void projectileSystem(Registry& registry, sparse_array<Position>& positions, sparse_array<Velocity>& velocities, sparse_array<Projectile>& projectiles, std::vector<size_t>& bullets) {
    for (size_t i = 0; i < projectiles.size(); ++i) {
        const auto& proj = projectiles[i];
        if (proj) {
            auto& pos = positions[i];
            const auto& vel = velocities[i];
            if (pos && vel) {
                pos->x += vel->vx * proj->speed;
                pos->y += vel->vy * proj->speed;
                std::cout << "Projectile " << i << " position: (" << pos->x << ", " << pos->y << ")" << std::endl;
                if (pos->x > 800) {
                    std::cout << "Killing entity " << i << std::endl;
                    registry.kill_entity(i);
                    bullets.push_back(i);

                }
            }
        }
    }
}

#endif // PROJECTILESYSTEM_H