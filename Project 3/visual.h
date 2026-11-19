#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

using namespace std;

struct Visit {
    string patient;
    string doctor;
};

struct DisplayState {
    int time;
    vector<string> waiting;
    vector<Visit> beingSeen;
};

class HospitalView {
    public:
        HospitalView(sf::RenderWindow& window, const sf::Font& font);
        void draw(const DisplayState& state);

    private:
        void drawClock(int time);
        void drawWaiting(const vector<string>& waiting);
        void drawBeingSeen(const vector<Visit>& beingSeen);
        void drawText(const string& str, sf::Vector2f pos, unsigned int size);

        sf::RenderWindow& window;
        const sf::Font& font;
    };

    DisplayState mockState();