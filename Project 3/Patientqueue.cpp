#include "PatientQueue.h"
#include "Patient.h"

#include <fstream>
#include <algorithm>

LoadResult PatientQueue::loadCSV(const std::string& path) {
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
        if (!line.empty() && line.back() == '\r') line.pop_back();      
        if (line.find_first_not_of(" \t") == std::string::npos) continue;   

        Patient p;
        if (!Patient::fromCSV(line, p)) {
            ++r.skipped;                                  
            if (r.firstBadLine == 0) r.firstBadLine = lineNo;
            continue;
        }
        admit(p);        
        ++r.loaded;
    }
    return r;
}

void PatientQueue::admit(Patient p) {
    p.arrival = nextArrival_++;
    heap_.push(p);
}

std::optional<Patient> PatientQueue::peekNext() const {
    if(heap_.empty()){
        return std::nullopt;
    }
    else{
        return heap_.top();
    }
    
}

std::optional<Patient> PatientQueue::takeNext() {
    if(heap_.empty()){
        return std::nullopt;
    }
    else{
        return heap_.pop();
    }
    
}

void PatientQueue::worsenAll(int amount, int worst) {
    heap_.updateAll([&](Patient& p) {
    p.condition = std::min(p.condition + amount, worst);
    });
}

std::vector<Patient> PatientQueue::waiting() const {
    // TODO: return heap_.items();   (a copy — the GUI must not hold references into the heap)
    return heap_.items();
}

std::size_t PatientQueue::count() const {
    return heap_.size();
}