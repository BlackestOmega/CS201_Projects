
#include "DoctorDB.h"
 
#include <fstream>
 
DoctorDB::DoctorDB(int minDegree) : tree_(minDegree) {}
 
LoadResult DoctorDB::loadCsv(const std::string& path) {
    LoadResult r;
    std::ifstream in(path);
    if (!in) {
        r.error = "could not open " + path;
        return r;
    }
 
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (!line.empty() && line.back() == '\r') line.pop_back();   // Windows line endings
        if (line.find_first_not_of(" \t") == std::string::npos) continue;  // blank line
 
        Doctor d;
        if (!Doctor::fromCsv(line, d) || !tree_.insert(d)) {
            ++r.skipped;                       // malformed, or duplicate key
            if (r.firstBadLine == 0) r.firstBadLine = lineNo;
            continue;
        }
        ++r.loaded;
    }
    return r;
}
 
bool DoctorDB::add(const Doctor& d) {
    return tree_.insert(d);
}
 
std::optional<Doctor> DoctorDB::find(const std::string& name, int age,
                                     const std::string& dob) const {
    Doctor key;          // only the key fields matter for comparison
    key.name = name;
    key.age = age;
    key.dob = dob;
    if (const Doctor* found = tree_.find(key)) return *found;   // copy out of the tree
    return std::nullopt;
}
 
std::vector<Doctor> DoctorDB::findByName(const std::string& name) const {
    std::vector<Doctor> out;
    tree_.forEach([&](const Doctor& d) {
        if (d.name == name) out.push_back(d);
    });
    return out;
}
 
std::vector<Doctor> DoctorDB::onShiftAt(const std::string& time12) const {
    std::vector<Doctor> out;
    int m = Doctor::toMinutes(time12);
    if (m < 0) return out;
    tree_.forEach([&](const Doctor& d) {
        if (d.onShift(m)) out.push_back(d);
    });
    return out;
}
 
std::vector<Doctor> DoctorDB::all() const {
    std::vector<Doctor> out;
    out.reserve(tree_.size());
    tree_.forEach([&](const Doctor& d) { out.push_back(d); });
    return out;
}
 