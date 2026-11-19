#include <SFML/Graphics.hpp>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

// Doctor includes
#include "BTree.h"
#include "DoctorDB.h"
#include "visual.h"


int main() {
    sf::VideoMode mode({1000, 600});
    sf::RenderWindow window(mode, "Nota Hospital");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("fonts/DejaVuSans.ttf")) {
        return 1;
    }

    HospitalView view(window, font);
    DisplayState state = mockState();
    sf::Color background(25, 25, 30);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Space) {
                    state.time += 5;
                }
            }
        }

        window.clear(background);
        view.draw(state);
        window.display();
    }
}
