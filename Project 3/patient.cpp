/**
 * @file patient.cpp
 * @author Original Author Daniel Healy finsihed by Matthew Carter
 * @brief 
 * 
 */
#include <string>
#include <iostream>

#include <cctype>
#include <ostream>
#include <sstream>
#include <tuple>
#include <vector>
#include "Patient.h"
using std::string;

/*
thinking space
the space for thinking

-want to hold a vector of patients
- that vector should become the heap?
- when a doctor is done seeing a patient the program will push the patient back into the heap
*/



/**
 * @brief 
 * Daniel's
 */
namespace {

// Remove spaces/tabs/\r from both ends of a string.
std::string trim(const std::string& s) {
    std::size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

}  // namespace

/**
 * @brief 
 * Daniel's (with changes)
 * @param o 
 * @return true 
 * @return false 
 */
bool Patient::operator<(const Patient& o) const {
    return std::tie(name, age, birthday) < std::tie(o.name, o.age, o.birthday);
}

// Daniel's (with changes)
bool Patient::operator==(const Patient& o) const {
    return std::tie(name, age, birthday) == std::tie(o.name, o.age, o.birthday);
}

bool TreatFirst::operator()(const Patient& a, const Patient& b) const {
    if (a.condition != b.condition) {
        return a.condition > b.condition;   // 10 = emergency, seen first
    }
    return a.arrival < b.arrival;           // same condition: first come, first served
}

// Daniel's
bool Patient::fromCSV(const std::string& line, Patient& out) {
    //CSV has 3 fields for patients
	std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) fields.push_back(trim(field));
    if (fields.size() != 3) return false;
    for (const auto& f : fields)
        if (f.empty()) return false;

    Patient p;
    p.name = fields[0];
    try {
        std::size_t used = 0;
        p.age = std::stoi(fields[1], &used);
        if (used != fields[1].size() || p.age < 0) return false;
    } catch (const std::exception&) {
        return false;
    }
    p.birthday = fields[2];
    
    out = p;   // only overwrite `out` once the whole line is valid
    return true;
}

//Daniel's
std::ostream& operator<<(std::ostream& os, const Patient& p) {
    return os << p.name << " (" << p.age << ", " << p.birthday << ") ";
}