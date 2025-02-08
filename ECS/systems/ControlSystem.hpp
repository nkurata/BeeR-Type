/*
** EPITECH PROJECT, 2024
** R-Type ECS
** File description:
** Control System
*/

#ifndef CONTROLSYSTEM_H
    #define CONTROLSYSTEM_H

#include "Registry.hpp"
#include "Velocity.hpp"
#include "Controllable.hpp"
#include <SFML/Window.hpp>
#include "CollisionSystem.hpp"

inline void controlSystem(Registry& registry, sparse_array<Velocity>& velocities, sparse_array<Controllable>& controllables, sparse_array<Position>& positions, sparse_array<Collidable>& collidables) {
    for (size_t i = 0; i < velocities.size() && i < controllables.size(); ++i) {
        auto& vel = velocities[i];
        auto& ctrl = controllables[i];
        if (vel && ctrl) {
            if (ctrl->moveRight) {
                vel->vx = 0.125f;
            } else if (ctrl->moveLeft) {
                vel->vx = -0.125f;
            } else {
                vel->vx = 0.0f;
            }

            if (ctrl->moveUp) {
                vel->vy = -0.125f;
            } else if (ctrl->moveDown) {
                vel->vy = 0.125f;
            } else {
                vel->vy = 0.0f;
            }
        }
    }
}

#endif // CONTROLSYSTEM_H
