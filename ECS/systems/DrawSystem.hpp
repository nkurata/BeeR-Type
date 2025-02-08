/*
** EPITECH PROJECT, 2024
** R-type ECS
** File description:
** Draw System
*/

#ifndef DRAWSYSTEM_H
    #define DRAWSYSTEM_H

#include "Registry.hpp"
#include "Position.hpp"
#include "Drawable.hpp"
#include <SFML/Graphics.hpp>

inline void drawSystem(Registry& registry, sf::RenderWindow& window, sparse_array<Position>& positions, sparse_array<Drawable>& drawables) {
    for (size_t i = 0; i < positions.size() && i < drawables.size(); ++i) {
        auto& pos = positions[i];
        auto& drawable = drawables[i];
        if (pos && drawable) {
            drawable->sprite.setPosition(pos->x, pos->y);
            window.draw(drawable->sprite);
        }
    }
}

#endif // DRAWSYSTEM_H
