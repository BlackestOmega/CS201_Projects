#pragma once
#include <iosfwd>
#include <string>

// EX row:  Whitney Hill,44,1982-04-30,Otolaryngology,10:00 PM,8:00 AM
struct Doctor {
    std::string name;
    int age = 0;
    std::string dob;          // YYYY-MM-DD
    std::string skill;
    std::string shiftStart;   // as in the file, e.g. "10:00 PM" (display)
    std::string shiftEnd;

    Doctor() = default;

    // Key = (name, age, dob). Skill and shift are NOT part of the key.
    bool operator<(const Doctor& o) const;
    bool operator==(const Doctor& o) const;

    // Parse one CSV line. false on a malformed line.
    static bool fromCsv(const std::string& line, Doctor& out);

    // "10:00 PM" -> 1320 (minutes since midnight), -1 if malformed.
    static int toMinutes(const std::string& time12);

    bool onShift(int minutesSinceMidnight) const;   // handles overnight shifts
};

std::ostream& operator<<(std::ostream& os, const Doctor& d);
