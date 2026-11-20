/**
 * @file Patient.h
 * @brief Patient record and its triage ordering.
 */
#pragma once

#include <iosfwd>
#include <string>

/**
 * @brief One patient. The key is (name, age, birthday); the rest is data.
 * Daniel's
 */
struct Patient {
    int condition=5;      ///< 0 = healthy, 10 = critical
    std::string name;
	int age;
	std::string birthday; ///< YYYY-MM-DD
    long arrival =0;      ///< queue order counter, set by PatientQueue::admit (tie-breaker)
    int arrivalTime = 0;  ///< minutes since midnight when the patient walks in
	Patient() = default;


	// compare by key only, so the B-tree can sort and find patients
	bool operator<(const Patient& o) const;
	bool operator==(const Patient& o) const;

	/**
	 * @brief Reads one CSV line into a Patient.
	 * @param line Either "name,age,dob" or "name,age,dob,condition,arrival time",
	 *             e.g. "Ann Lee,30,1996-01-01,7,8:05 AM". Condition is 0-10.
	 * @param out Receives the patient; left unchanged if the line is bad.
	 * @return true on success, false if the line is malformed.
	 */
	static bool fromCSV(const std::string& line, Patient& out);

	/// @return true if the patient's condition has reached 10 (critical).
	bool critical() const { return condition >= 10; }
};

/// Prints a patient as "Name (age, birthday)".
std::ostream& operator<<(std::ostream& os, const Patient& p);
 
/**
 * @brief Triage order for Heap<Patient, TreatFirst>.
 * Returns true if `a` should be seen before `b`:
 * worse condition first; equal condition -> earlier arrival first.
 */
struct TreatFirst {
    bool operator()(const Patient& a, const Patient& b) const;
};