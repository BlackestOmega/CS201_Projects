/**
 * @file PatientQueue.h
 * @brief Waiting-room API for the SFML front end (the patient-side twin of DoctorDB).
 *
 */
#pragma once


#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "DoctorDB.h"   // LoadResult
#include "Heap.h"
#include "Patient.h"

class PatientQueue {
public:
    PatientQueue() = default;

    /// Read patients from a CSV and admit each one. 
    LoadResult loadCSV(const std::string& path);

    /// Add a patient to the waiting room (also used to send a patient back after treatment).
    /// Stamps p.arrival with the next counter value before pushing.
    void admit(Patient p);

    /// Most urgent patient, without removing them. nullopt if nobody is waiting.
    std::optional<Patient> peekNext() const;

    /// Remove and return the most urgent patient (a doctor takes them). nullopt if empty.
    std::optional<Patient> takeNext();

    /// One time step: every waiting patient's condition worsens by `amount`,
    /// clamped to `worst` so it can't run past the scale.
    void worsenAll(int amount = 1, int worst = 10);

    /// Everyone waiting, in heap order (index 0 = next to be seen). For drawing.
    std::vector<Patient> waiting() const;

    std::size_t count() const;

private:
    Heap<Patient, TreatFirst> heap_;   ///< most urgent patient on top
    long nextArrival_ = 0;             ///< next value for Patient::arrival
};