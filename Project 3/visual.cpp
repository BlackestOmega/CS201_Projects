#include "visual.h"

using namespace std;

static string formatTime(int minutes) {
    int h = (minutes / 60) % 24;
    int m = minutes % 60;
    string hh = (h < 10 ? "0" : "") + to_string(h);
    string mm = (m < 10 ? "0" : "") + to_string(m);
    return hh + ":" + mm;
}

HospitalView::HospitalView(sf::RenderWindow& window, const sf::Font& font)
    : window(window), font(font) {}

void HospitalView::draw(const DisplayState& state) {
    drawClock(state.time);
    drawWaiting(state.waiting);
    drawBeingSeen(state.beingSeen);
}

void HospitalView::drawText(const string& str, sf::Vector2f pos, unsigned int size) {
    sf::Text text(font, str, size);
    text.setPosition(pos);
    window.draw(text);
}

void HospitalView::drawClock(int time) {
    string clock = "Time: " + formatTime(time);
    drawText(clock, {20.f, 15.f}, 28);
}

void HospitalView::drawWaiting(const vector<string>& waiting) {
    drawText("Waiting", {20.f, 70.f}, 22);
    for (size_t i = 0; i < waiting.size(); i++) {
        string line = to_string(i + 1) + ". " + waiting[i];
        sf::Vector2f pos(20.f, 110.f + i * 30.f);
        drawText(line, pos, 18);
    }
}

void HospitalView::drawBeingSeen(const vector<Visit>& beingSeen) {
    drawText("Being seen", {480.f, 70.f}, 22);
    for (size_t i = 0; i < beingSeen.size(); i++) {
        string line = beingSeen[i].patient + "  with  " + beingSeen[i].doctor;
        sf::Vector2f pos(480.f, 110.f + i * 30.f);
        drawText(line, pos, 18);
    }
}

DisplayState mockState() {
    DisplayState s;
    s.time = 8 * 60 + 30;
    s.waiting = {"Robert Mccann", "Karen Sellers", "Henry Knight", "Justin Morrison"};
    s.beingSeen = {
        {"Lawrence Macdonald", "Dan Mercer"},
        {"John Martin", "Briana Stewart"},
    };
    return s;
}
