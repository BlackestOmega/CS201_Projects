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
    /// 0 = healthy, 10 = critical, picks the circle's color
    int condition;
};

/// one patient in the waiting room and how sick they are
struct WaitingPatient {
    string name;
    /// 0 = healthy, 10 = critical, picks the circle's color
    int condition;
};

/// everything the window needs to draw one frame
struct DisplayState {
    int time;
    /// patients in the order they'll be seen
    vector<WaitingPatient> waiting;
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

        /// draws the lookup box and its results along the bottom
        /// @param query what the user has typed so far
        /// @param searched the name last searched for (empty if none)
        /// @param results lines from Hospital::lookup for that name
        /// output: nothing, draws to the window
        void drawLookup(const string& query, const string& searched,
                        const vector<string>& results);

    private:
        /// draws the clock in the top left
        /// @param time minutes since midnight
        /// output: nothing, draws the time as hh:mm
        void drawClock(int time);

        /// draws the numbered waiting list on the left side
        /// @param waiting patients in the order they'll be seen
        /// output: nothing, draws one line per patient with a condition circle
        void drawWaiting(const vector<WaitingPatient>& waiting);

        /// draws who is being seen by which doctor on the right side
        /// @param beingSeen current patient/doctor pairs
        /// output: nothing, draws one line per visit with a condition circle
        void drawBeingSeen(const vector<Visit>& beingSeen);

        /// draws one piece of text, used by all the other draw functions
        /// @param str text to show
        /// @param pos top left spot
        /// @param size font size
        /// output: nothing, draws the text to the window
        void drawText(const string& str, sf::Vector2f pos, unsigned int size);

        /// draws a circle colored by how sick a patient is
        /// @param condition 0 = healthy, 10 = critical
        /// @param pos top left spot
        /// output: nothing, draws the circle to the window
        void drawCircle(int condition, sf::Vector2f pos);

        /// draws the color key for the condition circles next to the clock
        /// output: nothing, draws one circle and label per color
        void drawLegend();

        // references so the view uses main's window and font instead of copies
        sf::RenderWindow& window;
        const sf::Font& font;
    };

    /// fake data so the window can be tested
    /// @return a display state with a few waiting patients and visits
    DisplayState mockState();
