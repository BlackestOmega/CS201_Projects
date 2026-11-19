// test_main.cpp — tests for BTree, Doctor, DoctorDB and SFML.
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

    testSfml();

    std::cout << "\n" << g_pass << " passed, " << g_fail << " failed\n";
    return g_fail == 0 ? 0 : 1;
}