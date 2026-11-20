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
/// - Claude (Anthropic AI assistant), used to assist with this project
///   https://claude.com/claude-code

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "Hospital.h"
#include "visual.h"

/// loads one CSV and reports how it went
/// @param what label for the message, e.g. "doctors"
/// @param r result from the load
/// @return true if the file opened
static bool report(const std::string& what, const LoadResult& r) {
    if (!r.ok()) {
        std::cerr << r.error << "\n";
        return false;
    }
    std::cout << "Loaded " << r.loaded << " " << what;
    if (r.skipped) std::cout << " (" << r.skipped << " bad lines skipped)";
    std::cout << "\n";
    return true;
}

/// opens the window and runs the dashboard until it's closed
/// @return 0 when the window closes, 1 if the font or a CSV can't be loaded
int main() {
    // start at 7:55 AM so the first press shows the 8:00 AM arrivals
    Hospital hospital(7 * 60 + 55);
    if (!report("doctors", hospital.loadDoctors("doctors.csv")) ||
        !report("patients", hospital.loadPatients("patient.csv"))) {
        return 1;
    }

    sf::VideoMode mode({1000, 600});
    sf::RenderWindow window(mode, "Nota Hospital");
    window.setFramerateLimit(60);

    sf::Font font;
    // nothing can be drawn without the font so just quit
    if (!font.openFromFile("fonts/DejaVuSans.ttf")) {
        return 1;
    }

    HospitalView view(window, font);
    sf::Color background(25, 25, 30);

    // runs once per frame until the window gets closed
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                // space moves time forward 5 minutes, the spec lets time move with a button
                if (key->code == sf::Keyboard::Key::Space) {
                    hospital.step(5);
                }
            }
        }

        window.clear(background);
        view.draw(hospital.display());
        window.display();
    }
}
