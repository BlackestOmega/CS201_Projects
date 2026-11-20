/**
 * @file Patient.h
 * @brief Patient record and its triage ordering.
 */
#pragma once

#include <iosfwd>
#include <string>

/**
 * @brief 
 * Daniel's
 */
struct Patient {
    int condition=5;
    std::string name;
	int age;
	std::string birthday;
    long arrival =0;
	Patient() = default;
    
	
	//comparisons
	bool operator<(const Patient& o) const;
	bool operator==(const Patient& o) const;
	
	//read a CSV line
	static bool fromCSV(const std::string& line, Patient& out);
};

std::ostream& operator<<(std::ostream& os, const Patient& p);
 
/**
 * @brief Triage order for Heap<Patient, TreatFirst>.
 * Returns true if `a` should be seen before `b`:
 * worse condition first; equal condition -> earlier arrival first.
 */
struct TreatFirst {
    bool operator()(const Patient& a, const Patient& b) const;
};