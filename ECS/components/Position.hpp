/*
** EPITECH PROJECT, 2024
** R-Type ECS
** File description:
** Position
*/

#ifndef POSITION_H
    #define POSITION_H

struct Position {
    float x;
    float y;

    Position(float x, float y) : x(x), y(y) {}
};

#endif // POSITION_H