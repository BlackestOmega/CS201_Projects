/// @file main.cpp
/// CS 201 - Project #3
/// Names:      Samuel Bozeman, Joseph Mire, Matthew Carter, Daniel Healy
/// Instructor: Don Yessick
/// TA:         Lucas Hasting
///
/// Simulates Nota Hospital using a B-Tree for patient and doctor data,
/// with an SFML window showing who is waiting and being seen.
///
/// Sources
/// - SFML 3 tutorials and documentation, for the window and drawing
///   https://www.sfml-dev.org/tutorials/3.1/
///   https://www.sfml-dev.org/documentation/3.1.0/
/// - DejaVu Sans font, for the text in the window
///   https://dejavu-fonts.github.io/

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

// Doctor includes
#include "BTree.h"
#include "DoctorDB.h"
#include "visual.h"


/// opens the window and runs the dashboard until it's closed
/// @return 0 when the window closes, 1 if the font can't be loaded
int main() {
    sf::VideoMode mode({1000, 600});
    sf::RenderWindow window(mode, "Nota Hospital");
    window.setFramerateLimit(60);

    sf::Font font;
    // nothing can be drawn without the font so just quit
    if (!font.openFromFile("fonts/DejaVuSans.ttf")) {
        return 1;
    }

    HospitalView view(window, font);
    DisplayState state = mockState();
    sf::Color background(25, 25, 30);

    // runs once per frame until the window gets closed
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                // space moves time forward 5 minutes, the spec lets time move with a button
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
