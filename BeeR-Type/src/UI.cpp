#include "UI.hpp"
#include <iostream>

UI::UI() {
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Error loading font" << std::endl;
    }
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setPosition(10, 10);

    timeText.setFont(font);
    timeText.setCharacterSize(20);
    timeText.setPosition(10, 40);
}

void UI::update(float score, float gameTime, const std::vector<Player>& players) {
    scoreText.setString("Score: " + std::to_string(score));
    timeText.setString("Time: " + std::to_string(gameTime));

    healthTexts.clear();
    for (const auto& player : players) {
        sf::Text healthText;
        healthText.setFont(font);
        healthText.setCharacterSize(20);
        healthText.setString("Health: " + std::to_string(player.getHealth()));
        healthText.setPosition(player.getPosition().x, player.getPosition().y - 20);
        healthTexts.push_back(healthText);
    }
}

void UI::render(sf::RenderWindow& window) {
    window.draw(scoreText);
    window.draw(timeText);
    for (const auto& healthText : healthTexts) {
        window.draw(healthText);
    }
}
