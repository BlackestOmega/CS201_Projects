/**
 * @file Doctor.h
 * @brief Doctor record: key fields, CSV parsing and shift-time helpers.
 */
#pragma once
#include <iosfwd>
#include <string>

/**
 * @brief One doctor, as read from a CSV line such as
 *        "Whitney Hill,44,1982-04-30,Otolaryngology,10:00 PM,8:00 AM".
 *
 * The key is (name, age, dob); skill and shift times are data, not key.
 */
struct Doctor {
    std::string name;
    int age = 0;
    std::string dob;          // YYYY-MM-DD
    std::string skill;
    std::string shiftStart;   // as in the file, e.g. "10:00 PM" (display)
    std::string shiftEnd;

    Doctor() = default;

   /**
 * @brief Compares doctors by key (name, age, dob) only.
 * @param o Doctor to compare against.
 * @return true if both have the same name, age and date of birth.
 */
    bool operator<(const Doctor& o) const;
    bool operator==(const Doctor& o) const;

    /**
 * @brief Parses one CSV line into a Doctor.
 * @param line A line with six comma-separated fields:
 *             name, age, dob, skill, shift start, shift end.
 * @param out Receives the parsed doctor; left unchanged if parsing fails.
 * @return true on success; false if the line is malformed.
 */
    static bool fromCsv(const std::string& line, Doctor& out);

    /**
 * @brief Converts a 12-hour time to minutes since midnight.
 * @param time12 Time such as "10:00 PM", "10:00PM" or "8:30 am".
 * @return Minutes since midnight (0-1439), or -1 if the text is not a valid time.
 */
    static int toMinutes(const std::string& time12);

    /**
 * @brief Whether the doctor is working at a given time of day. handles overnight shifts
 *
 *
 * @param minutesSinceMidnight Time to check, 0-1439 (see toMinutes).
 * @return true if the time falls within the shift.
 */
    bool onShift(int minutesSinceMidnight) const;    
};

/**
 * @brief Prints a doctor as "Name (age, dob) Skill start - end".
 * @param os Stream to write to.
 * @param d Doctor to print.
 * @return The same stream, for chaining.
 */
std::ostream& operator<<(std::ostream& os, const Doctor& d);
