#ifndef SPRITE_HPP
    #define SPRITE_HPP

#include <SFML/Graphics.hpp>

enum class SpriteType {
    Enemy,
    Boss,
    Player,
    Bullet,
    Background,
    Start_button
};

class SpriteElement {
public:
    int id;
    int hp;
    int score;
    float vx, vy;
    sf::Sprite sprite;
    sf::RectangleShape healthBar;
};

struct PacketData {
    int action;
    int server_id;
    float new_x;
    float new_y;
    float new_vx;
    float new_vy;
};

struct LagMeter {
    double packets_lost_ = 0;
    double ping = 0;
};

enum class SceneState {
    MAIN_MENU,
    GAME,
    EXIT
};

#endif
