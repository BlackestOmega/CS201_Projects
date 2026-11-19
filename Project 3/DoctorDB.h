#pragma once
// DoctorDB.h — the API for SFML code calls.

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "BTree.h"
#include "Doctor.h"

// Intializing the results of the loading the csv files
struct LoadResult {
    int loaded = 0;
    int skipped = 0;       // bad lines + duplicates
    int firstBadLine = 0;  // 1-based line number of the first bad line (0 = none)
    std::string error;     // set if the file couldn't be opened
    bool ok() const { return error.empty(); }
};

// Class declaration for Data calls on the Doctor csv
class DoctorDB {
public:
    explicit DoctorDB(int minDegree = 3);

    LoadResult loadCsv(const std::string& path);
    bool add(const Doctor& d);                        // false if key already exists

    std::optional<Doctor> find(const std::string& name, int age, const std::string& dob) const;   // exact key
    std::vector<Doctor> findByName(const std::string& name) const;
    std::vector<Doctor> onShiftAt(const std::string& time12) const;  // "3:00 PM"
    std::vector<Doctor> all() const;                  // sorted by key

    std::size_t count() const { return tree_.size(); }

private:
    BTree<Doctor> tree_;
};
