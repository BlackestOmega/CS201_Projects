/**
 * @file Hospital.cpp
 * @brief The simulation step: discharge, worsen, admit, assign.
 */
#include "Hospital.h"

#include <algorithm>
#include <cctype>
#include <fstream>

Hospital::Hospital(int startTime)
    : doctors_(kMinDegree), patients_(kMinDegree), clock_(startTime) {}

LoadResult Hospital::loadDoctors(const std::string& path) {
    return doctors_.loadCsv(path);
}

LoadResult Hospital::loadPatients(const std::string& path) {
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

        Patient p;
        // bad line, or the same (name, age, dob) twice
        if (!Patient::fromCSV(line, p) || !patients_.insert(p)) {
            ++r.skipped;
            if (r.firstBadLine == 0) r.firstBadLine = lineNo;
            continue;
        }
        ++r.loaded;
    }

    admitArrivals();   // anyone who arrives at or before the start time is already here
    return r;
}

void Hospital::step(int minutes) {
    int before = clock_;
    clock_ += minutes;

    finishVisits();

    // worsen once each time the clock passes a 15-minute mark, before new
    // arrivals come in so they don't get worse the moment they walk in
    if (clock_ / kWorsenEvery != before / kWorsenEvery) {
        waiting_.worsenAll(1, 10);
    }

    admitArrivals();
    assignDoctors();
}

void Hospital::finishVisits() {
    // remove every visit that's over; those patients leave the hospital
    auto over = [&](const Treatment& t) { return t.endsAt <= clock_; };
    discharged_ += std::count_if(visits_.begin(), visits_.end(), over);
    visits_.erase(std::remove_if(visits_.begin(), visits_.end(), over), visits_.end());
}

void Hospital::admitArrivals() {
    // arrival times are times of day, so only day one has arrivals
    int now = std::min(clock_, 24 * 60 - 1);
    if (now <= lastChecked_) return;

    // walk the patient B-tree for everyone who arrived since the last check;
    // several patients can share an arrival time and they all come in together
    patients_.forEach([&](const Patient& p) {
        if (p.arrivalTime > lastChecked_ && p.arrivalTime <= now) {
            waiting_.admit(p);
        }
    });
    lastChecked_ = now;
}

bool Hospital::busy(const Doctor& d) const {
    return std::any_of(visits_.begin(), visits_.end(),
                       [&](const Treatment& t) { return t.doctor == d; });
}

void Hospital::assignDoctors() {
    int timeOfDay = clock_ % (24 * 60);
    for (const Doctor& d : doctors_.all()) {
        if (waiting_.count() == 0) return;              // nobody left to see
        if (!d.onShift(timeOfDay) || busy(d)) continue; // off shift, or with a patient

        // the heap hands back the most urgent patient
        std::optional<Patient> next = waiting_.takeNext();
        visits_.push_back({*next, d, clock_ + d.visitMinutes()});
    }
}

std::optional<Patient> Hospital::findPatient(const std::string& name, int age,
                                             const std::string& birthday) const {
    Patient key;   // only the key fields matter for comparison
    key.name = name;
    key.age = age;
    key.birthday = birthday;
    if (const Patient* found = patients_.find(key)) return *found;   // copy out of the tree
    return std::nullopt;
}

// turns minutes into hh:mm (24 hour), same format as the window's clock
static std::string clockText(int minutes) {
    int h = (minutes / 60) % 24, m = minutes % 60;
    return (h < 10 ? "0" : "") + std::to_string(h) + ":" + (m < 10 ? "0" : "") + std::to_string(m);
}

// lowercase copy, so "john martin" matches "John Martin"
static std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::vector<std::string> Hospital::lookup(const std::string& name) const {
    std::vector<std::string> lines;
    std::string want = lower(name);
    std::vector<Patient> line = waiting_.waiting();

    // patients: walk the patient B-tree for matching names
    patients_.forEach([&](const Patient& p) {
        if (lower(p.name) != want) return;
        lines.push_back("Patient: " + p.name + ", age " + std::to_string(p.age) +
                        ", born " + p.birthday);

        // where are they now? check visits, then the waiting room
        std::string status;
        for (const Treatment& t : visits_) {
            if (t.patient == p) {
                status = "Being seen by " + t.doctor.name + " until " + clockText(t.endsAt) +
                         " (condition " + std::to_string(t.patient.condition) + ")";
            }
        }
        for (const Patient& w : line) {
            if (w == p) {
                status = "Waiting, condition " + std::to_string(w.condition);
                if (w.critical()) status += " CRITICAL";
            }
        }
        if (status.empty()) {
            status = p.arrivalTime > clock_ ? "Arrives at " + clockText(p.arrivalTime)
                                            : "Treated and sent home";
        }
        lines.push_back("    " + status);
    });

    // doctors: same idea with the doctor B-tree
    for (const Doctor& d : doctors_.all()) {
        if (lower(d.name) != want) continue;
        lines.push_back("Doctor: " + d.name + ", age " + std::to_string(d.age) + ", " + d.skill +
                        ", skill " + std::to_string(d.skillLevel) + " (" +
                        std::to_string(d.visitMinutes()) + " min visits)");

        std::string status = "Shift " + d.shiftStart + " - " + d.shiftEnd + ", ";
        if (!d.onShift(clock_ % (24 * 60))) {
            status += "off shift";
        } else {
            status += "on shift, available";
            for (const Treatment& t : visits_) {
                if (t.doctor == d) status = "Shift " + d.shiftStart + " - " + d.shiftEnd +
                                            ", seeing " + t.patient.name;
            }
        }
        lines.push_back("    " + status);
    }
    return lines;
}

DisplayState Hospital::display() const {
    DisplayState s;
    s.time = clock_;

    // the heap's array isn't fully sorted, so sort a copy in treatment order
    std::vector<Patient> line = waiting_.waiting();
    std::sort(line.begin(), line.end(), TreatFirst{});
    for (const Patient& p : line) {
        std::string row = p.name + " (" + std::to_string(p.condition) + ")";
        if (p.critical()) row += "  CRITICAL";
        s.waiting.push_back(row);
    }

    for (const Treatment& t : visits_) {
        std::string who = t.patient.name + " (" + std::to_string(t.patient.condition) + ")";
        s.beingSeen.push_back({who, t.doctor.name});
    }
    return s;
}
