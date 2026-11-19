#include "Doctor.h"

#include <cctype>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace {

// Remove spaces/tabs/\r from both ends of a string.
std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

}  // namespace

bool Doctor::operator<(const Doctor& o) const {
    return std::tie(name, age, dob) < std::tie(o.name, o.age, o.dob);
}

bool Doctor::operator==(const Doctor& o) const {
    return std::tie(name, age, dob) == std::tie(o.name, o.age, o.dob);
}

bool Doctor::fromCsv(const std::string& line, Doctor& out) {
    // Split on commas into exactly 6 fields.
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) fields.push_back(trim(field));
    if (fields.size() != 6) return false;
    for (const auto& f : fields)
        if (f.empty()) return false;

    Doctor d;
    d.name = fields[0];
    try {
        std::size_t used = 0;
        d.age = std::stoi(fields[1], &used);
        if (used != fields[1].size() || d.age < 0) return false;  // reject "44abc"
    } catch (const std::exception&) {
        return false;
    }
    d.dob = fields[2];
    d.skill = fields[3];
    d.shiftStart = fields[4];
    d.shiftEnd = fields[5];

    // Shift times must be parseable, or onShift() can't work.
    if (toMinutes(d.shiftStart) < 0 || toMinutes(d.shiftEnd) < 0) return false;

    out = d;   // only overwrite `out` once the whole line is valid
    return true;
}

int Doctor::toMinutes(const std::string& time12) {
    // Accepts "10:00 PM", "10:00PM", "8:30 am".
    std::string s = trim(time12);
    std::size_t colon = s.find(':');
    if (colon == std::string::npos || colon == 0 || s.size() < colon + 3) return -1;

    std::string hourPart = s.substr(0, colon);
    std::string minPart = s.substr(colon + 1, 2);
    std::string ampm = trim(s.substr(colon + 3));
    for (char& c : ampm) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    for (char c : hourPart + minPart)
        if (!std::isdigit(static_cast<unsigned char>(c))) return -1;

    int hour = std::stoi(hourPart);   // safe: digits only, checked above
    int minute = std::stoi(minPart);
    if (hour < 1 || hour > 12 || minute > 59) return -1;

    if (ampm == "AM") {
        if (hour == 12) hour = 0;          // 12:xx AM is just after midnight
    } else if (ampm == "PM") {
        if (hour != 12) hour += 12;        // 12:xx PM is just after noon
    } else {
        return -1;
    }
    return hour * 60 + minute;
}

bool Doctor::onShift(int m) const {
    int s = toMinutes(shiftStart);
    int e = toMinutes(shiftEnd);
    if (s < 0 || e < 0 || m < 0 || m >= 24 * 60) return false;

    if (s < e) return s <= m && m < e;     // day shift, e.g. 8:00 AM - 4:00 PM
    if (s > e) return m >= s || m < e;     // overnight, e.g. 10:00 PM - 8:00 AM
    return true;                           // start == end: treat as a 24-hour shift
}

std::ostream& operator<<(std::ostream& os, const Doctor& d) {
    return os << d.name << " (" << d.age << ", " << d.dob << ") "
              << d.skill << " " << d.shiftStart << " - " << d.shiftEnd;
}