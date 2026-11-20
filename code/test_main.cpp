// test_main.cpp — tests for BTree, Doctor, DoctorDB, Heap, Patient, PatientQueue and SFML.
//
// Build & run with:  make test        (or: make test conan)
// The SFML text/window checks run only when a display is available
// (WSLg or a Linux desktop); without one they are skipped, not failed.
//
// Every CHECK prints only on failure; the summary at the end says how many passed.

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "BTree.h"
#include "Doctor.h"
#include "DoctorDB.h"
#include "Heap.h"
#include "Patient.h"
#include "PatientQueue.h"

// ---------------------------------------------------------------------------
// Tiny test harness: keeps going after a failure so you see every problem.
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (cond) {                                                          \
            ++g_pass;                                                        \
        } else {                                                             \
            ++g_fail;                                                        \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ << "  "    \
                      << #cond << "\n";                                      \
        }                                                                    \
    } while (0)

static void section(const char* name) { std::cout << "== " << name << "\n"; }

// Collect a tree's keys in forEach order.
template <typename T>
static std::vector<T> collect(const BTree<T>& tree) {
    std::vector<T> out;
    tree.forEach([&](const T& v) { out.push_back(v); });
    return out;
}

// ===========================================================================
//  BTree<int>
// ===========================================================================

static void testEmptyTree() {
    section("empty tree");
    BTree<int> tree(2);
    CHECK(tree.empty());
    CHECK(tree.size() == 0);
    CHECK(tree.find(42) == nullptr);
    CHECK(collect(tree).empty());
}

static void testConstructorRejectsBadDegree() {
    section("constructor rejects t < 2");
    bool threw = false;
    try {
        BTree<int> bad(1);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}

static void testSingleNode() {
    section("single leaf (no splits), t = 3");
    BTree<int> tree(3);  // a node holds up to 5 keys
    for (int v : {30, 10, 50, 20, 40}) CHECK(tree.insert(v));
    CHECK(tree.size() == 5);
    CHECK((collect(tree) == std::vector<int>{10, 20, 30, 40, 50}));
    for (int v : {10, 20, 30, 40, 50}) {
        const int* p = tree.find(v);
        CHECK(p != nullptr);
        if (p) CHECK(*p == v);
    }
    CHECK(tree.find(35) == nullptr);
    CHECK(tree.find(0) == nullptr);
    CHECK(tree.find(99) == nullptr);
}

static void testDuplicates() {
    section("duplicates are rejected");
    BTree<int> tree(2);
    CHECK(tree.insert(7));
    CHECK(!tree.insert(7));
    CHECK(tree.size() == 1);
    for (int i = 0; i < 50; ++i) tree.insert(i);
    CHECK(tree.size() == 50);           // 7 counted once
    CHECK(!tree.insert(25));
    CHECK(tree.size() == 50);
}

static void testFirstSplit() {
    section("first root split, t = 2");
    BTree<int> tree(2);  // full at 3 keys; the 4th insert splits the root
    for (int v : {1, 2, 3, 4}) tree.insert(v);
    CHECK((collect(tree) == std::vector<int>{1, 2, 3, 4}));
    for (int v : {1, 2, 3, 4}) CHECK(tree.find(v) != nullptr);
}

static void testPrintLevels() {
    section("printLevels, t = 2, insert 1..10 (check by eye)");
    BTree<int> tree(2);
    for (int v = 1; v <= 10; ++v) tree.insert(v);
    std::cout << "  expected:\n"
                 "    [4]\n"
                 "    [2] [6 | 8]\n"
                 "    [1] [3] [5] [7] [9 | 10]\n"
                 "  got:\n";
    std::ostringstream os;
    tree.printLevels(os);
    std::istringstream lines(os.str());
    for (std::string line; std::getline(lines, line);) std::cout << "    " << line << "\n";
}

// Insert many values in a given order and compare against std::set.
static void stress(int t, std::vector<int> values, const char* label) {
    BTree<int> tree(t);
    std::set<int> ref;
    for (int v : values) {
        bool inserted = tree.insert(v);
        bool expected = ref.insert(v).second;
        if (inserted != expected) {
            CHECK(inserted == expected);
            std::cerr << "    (insert " << v << " in " << label << ", t=" << t << ")\n";
            return;
        }
    }
    CHECK(tree.size() == ref.size());
    CHECK(collect(tree) == std::vector<int>(ref.begin(), ref.end()));

    bool allFound = std::all_of(ref.begin(), ref.end(), [&](int v) {
        const int* p = tree.find(v);
        return p && *p == v;
    });
    CHECK(allFound);
    CHECK(tree.find(-1) == nullptr);
    CHECK(tree.find(1'000'000) == nullptr);
}

static void testStress() {
    section("stress: ascending / descending / random, t = 2, 3, 10");
    for (int t : {2, 3, 10}) {
        std::vector<int> asc(5000);
        for (int i = 0; i < 5000; ++i) asc[i] = i;
        std::vector<int> desc(asc.rbegin(), asc.rend());

        std::mt19937 rng(12345);  // fixed seed: same "random" every run
        std::vector<int> rnd(20000);
        for (int& v : rnd) v = static_cast<int>(rng() % 50000);  // includes duplicates

        stress(t, asc, "ascending");
        stress(t, desc, "descending");
        stress(t, rnd, "random");
    }
}

// ===========================================================================
//  Doctor
// ===========================================================================

static void testToMinutes() {
    section("Doctor::toMinutes");
    CHECK(Doctor::toMinutes("12:00 AM") == 0);
    CHECK(Doctor::toMinutes("8:00 AM") == 8 * 60);
    CHECK(Doctor::toMinutes("12:30 PM") == 12 * 60 + 30);
    CHECK(Doctor::toMinutes("10:00 PM") == 22 * 60);
    CHECK(Doctor::toMinutes("11:59 PM") == 23 * 60 + 59);
    CHECK(Doctor::toMinutes("9:15pm") == 21 * 60 + 15);
    CHECK(Doctor::toMinutes("13:00 PM") == -1);
    CHECK(Doctor::toMinutes("10:00") == -1);
    CHECK(Doctor::toMinutes("garbage") == -1);
    CHECK(Doctor::toMinutes("") == -1);
}

static void testFromCsv() {
    section("Doctor::fromCsv");
    Doctor d;
    CHECK(Doctor::fromCsv("Whitney Hill,44,1982-04-30,Otolaryngology,10:00 PM,8:00 AM", d));
    CHECK(d.name == "Whitney Hill");
    CHECK(d.age == 44);
    CHECK(d.dob == "1982-04-30");
    CHECK(d.skill == "Otolaryngology");
    CHECK(d.shiftStart == "10:00 PM");
    CHECK(d.shiftEnd == "8:00 AM");

    Doctor e;
    CHECK(Doctor::fromCsv("Whitney Hill,44,1982-04-30,Otolaryngology,10:00 PM,8:00 AM\r", e));
    CHECK(e.shiftEnd == "8:00 AM");  // trailing \r stripped

    Doctor bad;
    CHECK(!Doctor::fromCsv("too,few,fields", bad));
    CHECK(!Doctor::fromCsv("A,notanumber,1990-01-01,X,8:00 AM,4:00 PM", bad));
    CHECK(!Doctor::fromCsv("A,44abc,1990-01-01,X,8:00 AM,4:00 PM", bad));
    CHECK(!Doctor::fromCsv("A,44,1990-01-01,X,25:00 PM,4:00 PM", bad));
    CHECK(!Doctor::fromCsv("A,44,,X,8:00 AM,4:00 PM", bad));
    CHECK(!Doctor::fromCsv("", bad));
}

static void testOnShift() {
    section("Doctor::onShift");
    Doctor day, night;
    Doctor::fromCsv("Day Doc,40,1986-01-01,Cardiology,8:00 AM,4:00 PM", day);
    Doctor::fromCsv("Night Doc,40,1986-01-02,Cardiology,10:00 PM,8:00 AM", night);

    auto at = [](const char* t) { return Doctor::toMinutes(t); };
    CHECK(day.onShift(at("8:00 AM")));     // start is inclusive
    CHECK(day.onShift(at("3:59 PM")));
    CHECK(!day.onShift(at("4:00 PM")));    // end is exclusive
    CHECK(!day.onShift(at("11:00 PM")));

    CHECK(night.onShift(at("10:00 PM")));
    CHECK(night.onShift(at("11:30 PM")));
    CHECK(night.onShift(at("12:00 AM")));  // across midnight
    CHECK(night.onShift(at("3:00 AM")));
    CHECK(!night.onShift(at("8:00 AM")));
    CHECK(!night.onShift(at("12:00 PM")));
}

static void testDoctorOrdering() {
    section("Doctor key ordering (name, age, dob)");
    Doctor a, b, c;
    Doctor::fromCsv("Ann Lee,30,1996-01-01,X,8:00 AM,4:00 PM", a);
    Doctor::fromCsv("Ann Lee,30,1996-05-05,Y,8:00 AM,4:00 PM", b);   // same name+age
    Doctor::fromCsv("Ann Lee,31,1995-01-01,X,8:00 AM,4:00 PM", c);
    CHECK(a < b);          // dob breaks the tie
    CHECK(b < c);          // age before dob
    CHECK(!(a == b));

    Doctor a2 = a;
    a2.skill = "Different";
    CHECK(a == a2);        // skill isn't part of the key
}

// ===========================================================================
//  DoctorDB
// ===========================================================================

static void testDoctorDB() {
    section("DoctorDB load + queries");
    const char* path = "test_doctors.csv";
    {
        std::ofstream f(path);
        f << "Whitney Hill,44,1982-04-30,Otolaryngology,10:00 PM,8:00 AM\r\n"  // line 1
          << "Bob Day,50,1976-01-01,Cardiology,8:00 AM,4:00 PM\n"              // line 2
          << "Bob Day,50,1976-02-02,Pathology,12:00 PM,10:00 PM\n"             // line 3
          << "\n"                                                              // line 4 (blank)
          << "bad,line\n"                                                      // line 5
          << "Whitney Hill,44,1982-04-30,Otolaryngology,10:00 PM,8:00 AM\n";   // line 6 (dup)
    }

    DoctorDB db(2);
    LoadResult r = db.loadCsv(path);
    CHECK(r.ok());
    CHECK(r.loaded == 3);
    CHECK(r.skipped == 2);
    CHECK(r.firstBadLine == 5);
    CHECK(db.count() == 3);

    CHECK(db.find("Whitney Hill", 44, "1982-04-30").has_value());
    CHECK(!db.find("Whitney Hill", 45, "1982-04-30").has_value());
    CHECK(!db.find("Nobody", 1, "2000-01-01").has_value());

    CHECK(db.findByName("Bob Day").size() == 2);
    CHECK(db.findByName("Nobody").empty());

    auto late = db.onShiftAt("11:00 PM");
    CHECK(late.size() == 1);
    if (!late.empty()) CHECK(late[0].name == "Whitney Hill");
    CHECK(db.onShiftAt("3:00 PM").size() == 2);
    CHECK(db.onShiftAt("not a time").empty());

    auto all = db.all();
    CHECK(all.size() == 3);
    CHECK(std::is_sorted(all.begin(), all.end()));

    Doctor extra;
    Doctor::fromCsv("Zed Zane,60,1966-06-06,Urology,6:00 AM,2:00 PM", extra);
    CHECK(db.add(extra));
    CHECK(!db.add(extra));
    CHECK(db.count() == 4);

    std::remove(path);

    DoctorDB empty;
    CHECK(!empty.loadCsv("does_not_exist.csv").ok());
}

static void testRealFile() {
    section("your doctors.csv (if present)");
    std::ifstream probe("doctors.csv");
    if (!probe) {
        std::cout << "  (no doctors.csv here, skipped)\n";
        return;
    }
    DoctorDB db(3);
    LoadResult r = db.loadCsv("doctors.csv");
    std::cout << "  " << r.loaded << " loaded, " << r.skipped << " skipped";
    if (r.firstBadLine) std::cout << " (first bad line: " << r.firstBadLine << ")";
    std::cout << "\n";
    CHECK(r.ok());
    auto all = db.all();
    CHECK(all.size() == db.count());
    CHECK(std::is_sorted(all.begin(), all.end()));
}

// ===========================================================================
//  Heap
// ===========================================================================

static void testHeapEmpty() {
    section("Heap: empty");
    Heap<int, std::greater<int>> h;
    CHECK(h.empty());
    CHECK(h.size() == 0);
    CHECK(h.isValid());

    bool topThrew = false, popThrew = false;
    try { h.top(); } catch (const std::out_of_range&) { topThrew = true; }
    try { h.pop(); } catch (const std::out_of_range&) { popThrew = true; }
    CHECK(topThrew);
    CHECK(popThrew);
}

static void testHeapSmall() {
    section("Heap: small max-heap by hand");
    Heap<int, std::greater<int>> h;   // greater => biggest comes out first
    for (int v : {30, 10, 50, 20, 40}) {
        h.push(v);
        CHECK(h.isValid());
    }
    CHECK(h.size() == 5);
    CHECK(h.top() == 50);
    std::vector<int> out;
    while (!h.empty()) {
        out.push_back(h.pop());
        CHECK(h.isValid());
    }
    CHECK((out == std::vector<int>{50, 40, 30, 20, 10}));

    Heap<int, std::greater<int>> one;
    one.push(7);
    CHECK(one.pop() == 7);            // the 1-element pop case
    CHECK(one.empty());
}

static void testHeapMin() {
    section("Heap: min-heap with std::less");
    Heap<int, std::less<int>> h;      // less => smallest comes out first
    for (int v : {5, 3, 8, 1, 9, 2}) h.push(v);
    std::vector<int> out;
    while (!h.empty()) out.push_back(h.pop());
    CHECK((out == std::vector<int>{1, 2, 3, 5, 8, 9}));
}

static void testHeapStress() {
    section("Heap: random pushes/pops vs std::sort");
    std::mt19937 rng(2024);
    for (int n : {1, 2, 3, 10, 1000}) {
        Heap<int, std::greater<int>> h;
        std::vector<int> ref;
        bool validEveryStep = true;
        for (int i = 0; i < n; ++i) {
            int v = static_cast<int>(rng() % 100);   // includes duplicates
            h.push(v);
            ref.push_back(v);
            validEveryStep = validEveryStep && h.isValid();
        }
        std::sort(ref.rbegin(), ref.rend());         // descending
        std::vector<int> out;
        while (!h.empty()) {
            out.push_back(h.pop());
            validEveryStep = validEveryStep && h.isValid();
        }
        CHECK(validEveryStep);
        CHECK(out == ref);
    }
}

static void testHeapUpdateAll() {
    section("Heap: updateAll keeps it a valid heap");
    Heap<int, std::greater<int>> h;
    for (int i = 0; i < 200; ++i) h.push(i);
    h.updateAll([](int& x) { x = (x * 37) % 101; });   // scrambles the order
    CHECK(h.isValid());
    CHECK(h.size() == 200);
    int prev = 1'000'000;
    bool descending = true;
    while (!h.empty()) {
        int x = h.pop();
        descending = descending && x <= prev;
        prev = x;
    }
    CHECK(descending);
}

// ===========================================================================
//  Patient
// ===========================================================================

static void testPatientFromCsv() {
    section("Patient::fromCSV");
    Patient p;
    CHECK(Patient::fromCSV("Ann Lee,30,1996-01-01", p));
    CHECK(p.name == "Ann Lee");
    CHECK(p.age == 30);
    CHECK(p.birthday == "1996-01-01");
    CHECK(p.condition == 5);           // default: CSV has no condition column

    Patient q;
    CHECK(Patient::fromCSV("  Ann Lee , 30 , 1996-01-01\r", q));
    CHECK(q.name == "Ann Lee");        // trimmed
    CHECK(q.birthday == "1996-01-01");

    Patient bad;
    CHECK(!Patient::fromCSV("too,few", bad));
    CHECK(!Patient::fromCSV("a,b,c,d", bad));
    CHECK(!Patient::fromCSV("Ann,thirty,1996-01-01", bad));
    CHECK(!Patient::fromCSV("Ann,30abc,1996-01-01", bad));
    CHECK(!Patient::fromCSV("Ann,-1,1996-01-01", bad));
    CHECK(!Patient::fromCSV("Ann,30,", bad));
    CHECK(!Patient::fromCSV("", bad));
}

static void testPatientIdentity() {
    section("Patient identity ignores condition");
    Patient a, b;
    Patient::fromCSV("Ann Lee,30,1996-01-01", a);
    Patient::fromCSV("Ann Lee,30,1996-01-01", b);
    b.condition = 9;
    CHECK(a == b);                     // same person, different condition
    CHECK(!(a < b));
    CHECK(!(b < a));
    CHECK(a == a);

    Patient c;
    Patient::fromCSV("Ann Lee,30,1996-05-05", c);   // same name+age, other birthday
    CHECK(!(a == c));
    CHECK(a < c);
}

static void testTreatFirst() {
    section("TreatFirst (triage order)");
    TreatFirst first;
    Patient sick, okay;
    sick.condition = 9;
    okay.condition = 3;
    CHECK(first(sick, okay));          // worse condition goes first
    CHECK(!first(okay, sick));

    Patient early, late;
    early.condition = late.condition = 5;
    early.arrival = 1;
    late.arrival = 2;
    CHECK(first(early, late));         // tie -> earlier arrival
    CHECK(!first(late, early));
    CHECK(!first(early, early));       // nobody goes before themselves
}

// ===========================================================================
//  PatientQueue
// ===========================================================================

static void testPatientQueue() {
    section("PatientQueue load + triage");
    const char* path = "test_patients.csv";
    {
        std::ofstream f(path);
        f << "Ann Lee,30,1996-01-01\r\n"   // line 1
          << "Bob Day,50,1976-01-01\n"     // line 2
          << "\n"                          // line 3 (blank)
          << "bad line\n"                  // line 4
          << "Cy Fox,41,1985-07-07\n";     // line 5
    }

    PatientQueue q;
    CHECK(!q.peekNext().has_value());  // empty queue
    CHECK(!q.takeNext().has_value());

    LoadResult r = q.loadCSV(path);
    CHECK(r.ok());
    CHECK(r.loaded == 3);
    CHECK(r.skipped == 1);
    CHECK(r.firstBadLine == 4);
    CHECK(q.count() == 3);

    // Everyone starts at condition 5, so file order decides.
    auto next = q.peekNext();
    CHECK(next.has_value());
    if (next) CHECK(next->name == "Ann Lee");
    CHECK(q.count() == 3);             // peek doesn't remove

    std::vector<Patient> w = q.waiting();
    CHECK(w.size() == 3);
    if (!w.empty() && next) CHECK(w[0] == *next);   // index 0 = next to be seen

    // Worsen everyone; the cap stops at 10.
    q.worsenAll(3);
    w = q.waiting();
    CHECK(std::all_of(w.begin(), w.end(), [](const Patient& p) { return p.condition == 8; }));
    q.worsenAll(5);
    w = q.waiting();
    CHECK(std::all_of(w.begin(), w.end(), [](const Patient& p) { return p.condition == 10; }));

    // Ann is treated, improves, and comes back: she waits behind the sicker ones.
    auto ann = q.takeNext();
    CHECK(ann.has_value());
    if (ann) {
        CHECK(ann->name == "Ann Lee");
        CHECK(q.count() == 2);
        ann->condition = 2;
        q.admit(*ann);
    }
    CHECK(q.count() == 3);

    std::vector<std::string> order;
    while (auto p = q.takeNext()) order.push_back(p->name);
    CHECK((order == std::vector<std::string>{"Bob Day", "Cy Fox", "Ann Lee"}));
    CHECK(q.count() == 0);

    // Same condition: someone re-admitted later goes behind someone already waiting.
    Patient x, y;
    Patient::fromCSV("X Ray,20,2006-01-01", x);
    Patient::fromCSV("Yu Zed,21,2005-01-01", y);
    q.admit(x);
    q.admit(y);
    auto first = q.takeNext();
    CHECK(first.has_value());
    if (first) CHECK(first->name == "X Ray");

    std::remove(path);

    PatientQueue missing;
    CHECK(!missing.loadCSV("does_not_exist.csv").ok());
}

static void testRealPatientFile() {
    section("your patient.csv (if present)");
    std::ifstream probe("patient.csv");
    if (!probe) {
        std::cout << "  (no patient.csv here, skipped)\n";
        return;
    }
    PatientQueue q;
    LoadResult r = q.loadCSV("patient.csv");
    std::cout << "  " << r.loaded << " loaded, " << r.skipped << " skipped";
    if (r.firstBadLine) std::cout << " (first bad line: " << r.firstBadLine << ")";
    std::cout << "\n";
    CHECK(r.ok());
    CHECK(q.count() == static_cast<std::size_t>(r.loaded));

    // Popping everything must never go from a better condition to a worse one.
    int prev = 11;
    bool ordered = true;
    while (auto p = q.takeNext()) {
        ordered = ordered && p->condition <= prev;
        prev = p->condition;
    }
    CHECK(ordered);
}

// ===========================================================================
//  SFML
// ===========================================================================

static void testSfml() {
    section("SFML: font");
    sf::Font font;
    bool fontOk = false;
    for (const char* p : {"DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"}) {
        if (std::filesystem::exists(p) && font.openFromFile(p)) {
            fontOk = true;
            break;
        }
    }
    CHECK(fontOk);

    // Everything below needs a display: even measuring text makes SFML create an
    // OpenGL texture for the glyphs, and without a display SFML aborts the program.
    if (!std::getenv("DISPLAY") && !std::getenv("WAYLAND_DISPLAY")) {
        std::cout << "  (no display - text and window tests skipped)\n";
        return;
    }

    section("SFML: text");
    if (fontOk) {
        sf::Text text(font, "Whitney Hill", 20);
        CHECK(text.getLocalBounds().size.x > 0);
        CHECK(text.getLocalBounds().size.y > 0);
    }

    section("SFML: window draws");
    sf::RenderWindow window(sf::VideoMode({200, 100}), "test");
    CHECK(window.isOpen());

    // Draw a red square, then read the pixels back to prove it actually rendered.
    sf::RectangleShape box({50.f, 50.f});
    box.setFillColor(sf::Color::Red);
    box.setPosition({10.f, 10.f});
    window.clear(sf::Color::Black);
    window.draw(box);

    sf::Texture shot(window.getSize());
    shot.update(window);                       // copy what was drawn
    sf::Image img = shot.copyToImage();
    CHECK(img.getPixel({20, 20}) == sf::Color::Red);     // inside the square
    CHECK(img.getPixel({150, 80}) == sf::Color::Black);  // background
    window.display();

    window.close();
    CHECK(!window.isOpen());
}

// ===========================================================================

int main() {
    testEmptyTree();
    testConstructorRejectsBadDegree();
    testSingleNode();
    testDuplicates();
    testFirstSplit();
    testPrintLevels();
    testStress();

    testToMinutes();
    testFromCsv();
    testOnShift();
    testDoctorOrdering();

    testDoctorDB();
    testRealFile();

    testHeapEmpty();
    testHeapSmall();
    testHeapMin();
    testHeapStress();
    testHeapUpdateAll();

    testPatientFromCsv();
    testPatientIdentity();
    testTreatFirst();

    testPatientQueue();
    testRealPatientFile();

    testSfml();

    std::cout << "\n" << g_pass << " passed, " << g_fail << " failed\n";
    return g_fail == 0 ? 0 : 1;
}