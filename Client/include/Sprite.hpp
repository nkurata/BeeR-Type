#ifndef SPRITE_TYPE_HPP
#define SPRITE_TYPE_HPP

#include <SFML/Graphics.hpp>

enum class SpriteType {
    BackgroundSprite,
    PlayerSprite,
    EnemySprite,
    BulletSprite,
    BossSprite,
    LobbyBackground,
    GameStart1,
    GameStart2,
    PlayerIcon,
};

class SpriteElement {
public:
    sf::Sprite sprite;
    int id;
};

#endif // SPRITE_TYPE_HPP
