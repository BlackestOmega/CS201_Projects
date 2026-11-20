/**
 * @file visual.h
 * @brief SFML dashboard that draws the hospital: clock, waiting list, and who is being seen.
 */
#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

using namespace std;

/// one patient being seen and the doctor seeing them
struct Visit {
    string patient;
    string doctor;
};

/// everything the window needs to draw one frame
struct DisplayState {
    int time;
    /// patients in the order they'll be seen
    vector<string> waiting;
    vector<Visit> beingSeen;
};

/// draws the hospital dashboard (clock, waiting list, being seen list)
class HospitalView {
    public:
        /// sets up the view to draw into a window with a font
        /// @param window window to draw into
        /// @param font font for all the text
        /// output: a new hospital view
        HospitalView(sf::RenderWindow& window, const sf::Font& font);

        /// draws the whole dashboard for one frame
        /// @param state current time, waiting patients, and visits
        /// output: nothing, draws to the window
        void draw(const DisplayState& state);

    private:
        /// draws the clock in the top left
        /// @param time minutes since midnight
        /// output: nothing, draws the time as hh:mm
        void drawClock(int time);

        /// draws the numbered waiting list on the left side
        /// @param waiting patient names in the order they'll be seen
        /// output: nothing, draws one line per patient
        void drawWaiting(const vector<string>& waiting);

        /// draws who is being seen by which doctor on the right side
        /// @param beingSeen current patient/doctor pairs
        /// output: nothing, draws one line per visit
        void drawBeingSeen(const vector<Visit>& beingSeen);

        /// draws one piece of text, used by all the other draw functions
        /// @param str text to show
        /// @param pos top left spot
        /// @param size font size
        /// output: nothing, draws the text to the window
        void drawText(const string& str, sf::Vector2f pos, unsigned int size);

        // references so the view uses main's window and font instead of copies
        sf::RenderWindow& window;
        const sf::Font& font;
    };

    /// fake data so the window can be tested
    /// @return a display state with a few waiting patients and visits
    DisplayState mockState();
