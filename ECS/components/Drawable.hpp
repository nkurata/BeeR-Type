/*
** EPITECH PROJECT, 2024
** R-Type ECS
** File description:
** Drawable
*/

#ifndef DRAWABLE_H
    #define DRAWABLE_H

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

struct Drawable {
    sf::Sprite sprite;
    sf::Texture texture;
    std::string path;

    Drawable(std::string path) : path(path) {
        if (!texture.loadFromFile(path)) {
            throw std::runtime_error("Failed to load texture from file: " + path);
        }
        sprite.setTexture(texture);
    }
};

#endif // DRAWABLE_H
