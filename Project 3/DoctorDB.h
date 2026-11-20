/**
 * @file DoctorDB.h
 * @brief Doctor database API used by the SFML front end.
 *
 * Wraps a BTree<Doctor>. Does no printing and no drawing; every query returns
 * copies so the GUI never holds pointers into the tree.
 */
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "BTree.h"
#include "Doctor.h"


/**
 * @brief Summary of a CSV load.
 */
struct LoadResult {
    int loaded = 0;
    int skipped = 0;       // bad lines + duplicates
    int firstBadLine = 0;  // 1-based line number of the first bad line (0 = none)
    std::string error;     // set if the file couldn't be opened

     /**
     * @brief Whether the file was opened successfully.
     * @return true if error is empty.
     */
    bool ok() const { return error.empty(); }
};

/**
 * @brief In-memory doctor database backed by a B-tree keyed on (name, age, dob).
 */
class DoctorDB {
public:
      /**
     * @brief Creates an empty database.
     * @param minDegree Minimum degree t for the underlying B-tree.
     */
    explicit DoctorDB(int minDegree = 3);

    /**
     * @brief Loads doctors from a CSV file.
     *
     * Skips blank lines, strips Windows line endings, and counts malformed lines
     * and duplicate keys as skipped.
     *
     * @param path Path to the CSV file.
     * @return Counts of loaded and skipped records, plus an error if the file
     *         could not be opened.
     */
    LoadResult loadCsv(const std::string& path);

    /**
     * @brief Adds one doctor.
     * @param d Doctor to add.
     * @return true if added; false if a doctor with the same key already exists.
     */
    bool add(const Doctor& d);       
    
    /**
     * @brief Finds a doctor by exact key.
     * @param name Full name.
     * @param age Age in years.
     * @param dob Date of birth, YYYY-MM-DD.
     * @return A copy of the doctor, or std::nullopt if not found.
     */
    std::optional<Doctor> find(const std::string& name, int age, const std::string& dob) const; 
    
    /**
     * @brief Finds every doctor with the given name.
     * @param name Full name to match exactly.
     * @return Matching doctors in key order; empty if none.
     */
    std::vector<Doctor> findByName(const std::string& name) const;

    /**
     * @brief Finds every doctor working at the given time.
     * @param time12 Time of day such as "3:00 PM".
     * @return Doctors on shift at that time; empty if none or if the time is invalid.
     */
    std::vector<Doctor> onShiftAt(const std::string& time12) const;

    /**
     * @brief Returns every doctor.
     * @return All doctors in key order.
     */
    std::vector<Doctor> all() const;

    /**
     * @brief Number of doctors stored.
     * @return Doctor count.
     */
    std::size_t count() const { return tree_.size(); }

private:
    BTree<Doctor> tree_; ///< Doctors keyed on (name, age, dob).
};
