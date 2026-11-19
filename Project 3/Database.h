#pragma once
// Database.h — the backend API the SFML front end calls.
//
// Rules for this layer:
//   * NO printing, NO SFML. It returns data; the GUI decides how to show it.
//   * Queries return COPIES (std::vector / std::optional), never pointers into the
//     tree — a later insert can split nodes and move records, which would leave a
//     stored pointer dangling while the GUI is still displaying it.

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "BTree.h"
#include "Doctor.h"

struct LoadResult {
    int loaded = 0;        // records inserted
    int skipped = 0;       // malformed lines or duplicates
    std::string error;     // non-empty if the file couldn't be opened
    bool ok() const { return error.empty(); }
};

class Database {
public:
    explicit Database(int minDegree);

    // ---- loading / adding ----
    LoadResult loadPatients(const std::string& path);
    LoadResult loadDoctors(const std::string& path);
    bool addDoctor(const Doctor& d);

    // ---- queries ----
    std::vector<Doctor> findDoctorsByName(const std::string& name) const;
    std::vector<Doctor> doctorsOnShift(int hour) const;
    std::vector<Doctor> doctorsBySkill(const std::string& skill) const;
    std::vector<Doctor> allDoctors() const;

    // ---- stats / debugging ----
    std::size_t doctorCount() const { return doctors_.size(); }
    bool checkAll() const;   // both trees pass checkInvariants

    // ---- read-only access for drawing the trees ----
    const BTree<Doctor>& doctorTree() const { return doctors_; }

private:
    BTree<Doctor> doctors_;
};
