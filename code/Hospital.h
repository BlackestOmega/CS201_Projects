/**
 * @file Hospital.h
 * @brief The simulation: ties the doctor and patient B-trees, the waiting-room
 *        heap and the clock together.
 */
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "BTree.h"
#include "Doctor.h"
#include "DoctorDB.h"
#include "Patient.h"
#include "PatientQueue.h"
#include "visual.h"

/**
 * @brief One patient currently being seen by a doctor.
 */
struct Treatment {
    Patient patient;
    Doctor doctor;
    int endsAt;   ///< clock time (minutes) when the visit is over
};

/**
 * @brief Runs Nota Hospital one time step at a time.
 *
 * Every step:
 *  1. visits that are over end and those patients go home,
 *  2. every 15 minutes, everyone still waiting gets 1 worse (10 = critical),
 *  3. patients whose arrival time has come enter the waiting room,
 *  4. each doctor who is on shift and free takes the most urgent patient.
 */
class Hospital {
public:
    static constexpr int kMinDegree = 3;      ///< t for both B-trees
    static constexpr int kWorsenEvery = 15;   ///< minutes between condition drops

    /**
     * @brief Creates an empty hospital.
     * @param startTime Starting clock time in minutes since midnight.
     */
    explicit Hospital(int startTime);

    /**
     * @brief Loads doctors into the doctor B-tree.
     * @param path CSV file, one doctor per line.
     * @return Counts of loaded and skipped lines.
     */
    LoadResult loadDoctors(const std::string& path);

    /**
     * @brief Loads patients into the patient B-tree. Patients enter the waiting
     *        room later, once the clock reaches their arrival time.
     * @param path CSV file, one patient per line.
     * @return Counts of loaded and skipped lines.
     */
    LoadResult loadPatients(const std::string& path);

    /**
     * @brief Moves time forward and runs one round of the simulation.
     * @param minutes How far to move the clock.
     */
    void step(int minutes);

    /**
     * @brief Builds what the window should show right now.
     * @return Clock, waiting list (most urgent first) and current visits.
     */
    DisplayState display() const;

    /**
     * @brief Looks up a patient record in the patient B-tree.
     * @param name Full name.
     * @param age Age in years.
     * @param birthday Date of birth, YYYY-MM-DD.
     * @return A copy of the record, or std::nullopt if there is none.
     */
    std::optional<Patient> findPatient(const std::string& name, int age,
                                       const std::string& birthday) const;

    /// @return Current clock time in minutes since the start of day one.
    int time() const { return clock_; }

    /// @return How many patients have been treated and sent home.
    std::size_t dischargedCount() const { return discharged_; }

private:
    void finishVisits();                  // step 1
    void admitArrivals();                 // step 3
    void assignDoctors();                 // step 4
    bool busy(const Doctor& d) const;     // is d already seeing someone?

    DoctorDB doctors_;                    ///< doctor records (B-tree)
    BTree<Patient> patients_;             ///< patient records (B-tree)
    PatientQueue waiting_;                ///< waiting room, most urgent on top (heap)
    std::vector<Treatment> visits_;       ///< patients being seen right now
    int clock_;                           ///< minutes since midnight of day one
    int lastChecked_ = -1;                ///< arrivals up to this time are already in
    std::size_t discharged_ = 0;
};
