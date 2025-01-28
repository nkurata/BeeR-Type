#ifndef UI_HPP
#define UI_HPP

#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include <vector>

class UI {
public:
    UI();
    void update(float score, float gameTime, const std::vector<Player>& players);
    void render(sf::RenderWindow& window);

private:
    sf::Font font;
    sf::Text scoreText;
    sf::Text timeText;
    std::vector<sf::Text> healthTexts;
};

#endif // UI_HPP
