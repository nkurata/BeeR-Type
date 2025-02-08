/*
** EPITECH PROJECT, 2024
** R-Type ECS
** File description:
** Velocity
*/

#ifndef VELOCITY_H
    #define VELOCITY_H

struct Velocity {
    float vx, vy;

    Velocity(float dx = 0.0f, float dy = 0.0f) : vx(dx), vy(dy) {}
};

#endif // VELOCITY_H