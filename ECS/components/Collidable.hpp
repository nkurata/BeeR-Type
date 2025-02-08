/*
** EPITECH PROJECT, 2024
** CPP-Rtype
** File description:
** Collidable
*/

#ifndef COLLIDABLE_H_
    #define COLLIDABLE_H_

    struct Hitbox {
        float width;
        float height;
    };

    struct Collidable {
        bool is_collidable = true;
        Hitbox hitbox;
    };

#endif /* !COLLIDABLE_H_ */
