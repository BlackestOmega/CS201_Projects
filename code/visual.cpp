/**
 * @file visual.cpp
 * @brief SFML drawing for the hospital dashboard.
 */
#include "visual.h"

using namespace std;

/// turns minutes since midnight into a 24 hour clock string
/// @param minutes minutes since midnight
/// @return the time as hh:mm
static string formatTime(int minutes) {
    // split into hours and minutes, % 24 wraps back to 00 after midnight
    int h = (minutes / 60) % 24;
    int m = minutes % 60;
    string hh = (h < 10 ? "0" : "") + to_string(h);
    string mm = (m < 10 ? "0" : "") + to_string(m);
    return hh + ":" + mm;
}

/// picks the circle color for a condition level
/// @param condition 0 = healthy, 10 = critical
/// @return green, yellow, orange, or red as the patient gets sicker
static sf::Color conditionColor(int condition) {
    if (condition <= 3) return sf::Color::Green;
    if (condition <= 6) return sf::Color::Yellow;
    // sfml has no built in orange
    if (condition <= 9) return sf::Color(255, 140, 0);
    // only 10 is red since the spec says 10 is critical
    return sf::Color::Red;
}

HospitalView::HospitalView(sf::RenderWindow& window, const sf::Font& font)
    : window(window), font(font) {}

void HospitalView::draw(const DisplayState& state) {
    drawClock(state.time);
    drawLegend();
    drawWaiting(state.waiting);
    drawBeingSeen(state.beingSeen);
}

void HospitalView::drawText(const string& str, sf::Vector2f pos, unsigned int size) {
    sf::Text text(font, str, size);
    text.setPosition(pos);
    window.draw(text);
}

void HospitalView::drawCircle(int condition, sf::Vector2f pos) {
    sf::CircleShape circle(7.f);
    circle.setFillColor(conditionColor(condition));
    circle.setPosition(pos);
    window.draw(circle);
}

void HospitalView::drawLegend() {
    // one sample condition from each color range, 80px apart, to the right of the clock
    drawCircle(0, {480.f, 25.f});
    drawText("0-3", {500.f, 20.f}, 18);
    drawCircle(4, {560.f, 25.f});
    drawText("4-6", {580.f, 20.f}, 18);
    drawCircle(7, {640.f, 25.f});
    drawText("7-9", {660.f, 20.f}, 18);
    drawCircle(10, {720.f, 25.f});
    drawText("10 critical", {740.f, 20.f}, 18);
}

void HospitalView::drawClock(int time) {
    string clock = "Time: " + formatTime(time);
    drawText(clock, {20.f, 15.f}, 28);
}

void HospitalView::drawWaiting(const vector<WaitingPatient>& waiting) {
    drawText("Waiting", {20.f, 70.f}, 22);
    for (size_t i = 0; i < waiting.size(); i++) {
        // i + 1 so the list starts at 1 instead of 0
        string line = to_string(i + 1) + ". " + waiting[i].name;
        // each row goes 30px below the last one, starting under the header
        float y = 110.f + i * 30.f;
        // circle goes first, +5 so it lines up with the middle of the text
        drawCircle(waiting[i].condition, {20.f, y + 5.f});
        drawText(line, {42.f, y}, 18);
    }
}

void HospitalView::drawBeingSeen(const vector<Visit>& beingSeen) {
    drawText("Being seen", {480.f, 70.f}, 22);
    for (size_t i = 0; i < beingSeen.size(); i++) {
        string line = beingSeen[i].patient + "  with  " + beingSeen[i].doctor;
        // same row spacing as the waiting list so the two columns line up
        float y = 110.f + i * 30.f;
        drawCircle(beingSeen[i].condition, {480.f, y + 5.f});
        drawText(line, {502.f, y}, 18);
    }
}

void HospitalView::drawLookup(const string& query, const string& searched,
                              const vector<string>& results) {
    // the panel starts below the two lists, which fit 15 rows above y = 600
    float top = 610.f;
    if (query.empty() && searched.empty()) {
        drawText("Lookup: Type full first and last name and press enter (Esc clears).", {20.f, top}, 20);
        return;
    }
    // "_" acts as a cursor at the end of the typed text
    drawText("Lookup: " + query + "_", {20.f, top}, 20);

    if (searched.empty()) return;
    if (results.empty()) {
        drawText("No patient or doctor named " + searched, {20.f, top + 35.f}, 18);
        return;
    }
    for (size_t i = 0; i < results.size(); i++) {
        drawText(results[i], {20.f, top + 35.f + i * 26.f}, 18);
    }
}

DisplayState mockState() {
    DisplayState s;
    // 8:30 am in minutes
    s.time = 8 * 60 + 30;
    // placeholder names until the backend fills in the real patients and doctors
    s.waiting = {{"Robert Mccann", 10}, {"Karen Sellers", 8}, {"Henry Knight", 5}, {"Justin Morrison", 2}};
    s.beingSeen = {
        {"Lawrence Macdonald", "Dan Mercer", 9},
        {"John Martin", "Briana Stewart", 4},
    };
    return s;
}
