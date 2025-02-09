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

inline void controlSystem(Registry& registry, sparse_array<Velocity>& velocities, sparse_array<Controllable>& controllables) {
    for (size_t i = 0; i < velocities.size() && i < controllables.size(); ++i) {
        auto vel = velocities[i];
        auto ctrl = controllables[i];

        if (vel && ctrl) {
            vel->vx = 0.0f;
            vel->vy = 0.0f;

            if (ctrl->moveRight) vel->vx += 10.0f;
            if (ctrl->moveLeft) vel->vx -= 10.0f;
            if (ctrl->moveUp) vel->vy -= 10.0f;
            if (ctrl->moveDown) vel->vy += 10.0f;
        }
    }
}

#endif // CONTROLSYSTEM_H
