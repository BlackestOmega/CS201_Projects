/// @file main.cpp
/// CS 201 - Project #3
/// Names:      Samuel Bozeman, Joseph Mire, Matthew Carter, Daniel Healy
/// Instructor: Don Yessick
/// TA:         Lucas Hasting
///
/// Simulates Nota Hospital using a B-Tree for patient and doctor data,
/// with an SFML window showing who is waiting and being seen.
/// Doctors and patients are loaded from doctors.csv and patient.csv.
/// Waiting patients sit in a heap ordered by condition (10 = critical),
/// and get 1 worse every 15 minutes until a doctor sees them.
/// Doctors only see patients during their shift, and higher skill
/// doctors finish visits faster. Press Space to move time forward 5 minutes.
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

    // 600px for the lists, 200px below them for the lookup panel
    sf::VideoMode mode({1000, 800});
    sf::RenderWindow window(mode, "Nota Hospital");
    window.setFramerateLimit(60);

    sf::Font font;
    // nothing can be drawn without the font so just quit
    if (!font.openFromFile("fonts/DejaVuSans.ttf")) {
        return 1;
    }

    HospitalView view(window, font);
    sf::Color background(25, 25, 30);

    std::string query;      // name being typed in the lookup box
    std::string searched;   // name last searched for with Enter

    // runs once per frame until the window gets closed
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                // space moves time forward 5 minutes, the spec lets time move with a button.
                // while typing a name, space is part of the name instead
                if (key->code == sf::Keyboard::Key::Space && query.empty()) {
                    hospital.step(5);
                } else if (key->code == sf::Keyboard::Key::Enter) {
                    searched = query;
                    query.clear();      // empty box again, so space moves time again
                } else if (key->code == sf::Keyboard::Key::Backspace && !query.empty()) {
                    query.pop_back();
                } else if (key->code == sf::Keyboard::Key::Escape) {
                    query.clear();
                    searched.clear();
                }
            } else if (const auto* text = event->getIf<sf::Event::TextEntered>()) {
                // add typed letters to the box; skip control keys and a leading space
                char32_t c = text->unicode;
                if (c >= 32 && c < 127 && !(c == ' ' && query.empty())) {
                    query += static_cast<char>(c);
                }
            }
        }

        window.clear(background);
        view.draw(hospital.display());
        // re-run the search every frame so the status stays current as time moves
        view.drawLookup(query, searched, searched.empty() ? std::vector<std::string>{}
                                                          : hospital.lookup(searched));
        window.display();
    }
}
