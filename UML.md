# UML Class Diagram — Nota Hospital Simulation

```mermaid
classDiagram
    direction TB

    class Hospital {
        +int kMinDegree$ = 3
        +int kWorsenEvery$ = 15
        -DoctorDB doctors_
        -BTree~Patient~ patients_
        -PatientQueue waiting_
        -vector~Treatment~ visits_
        -int clock_
        -int lastChecked_
        -size_t discharged_
        +Hospital(int startTime)
        +loadDoctors(string path) LoadResult
        +loadPatients(string path) LoadResult
        +step(int minutes) void
        +display() DisplayState
        +findPatient(string name, int age, string birthday) optional~Patient~
        +time() int
        +dischargedCount() size_t
        -finishVisits() void
        -admitArrivals() void
        -assignDoctors() void
        -busy(Doctor d) bool
    }

    class Treatment {
        +Patient patient
        +Doctor doctor
        +int endsAt
    }

    class DoctorDB {
        -BTree~Doctor~ tree_
        +DoctorDB(int minDegree = 3)
        +loadCsv(string path) LoadResult
        +add(Doctor d) bool
        +find(string name, int age, string dob) optional~Doctor~
        +findByName(string name) vector~Doctor~
        +onShiftAt(string time12) vector~Doctor~
        +all() vector~Doctor~
        +count() size_t
    }

    class BTree~T~ {
        -Node* root_
        -int t_
        -size_t count_
        +BTree(int minDegree)
        +insert(T k) bool
        +find(T k) T*
        +forEach(function visit) void
        +size() size_t
        +empty() bool
        +printLevels(ostream os) void
        -search(Node* x, T k) T*
        -splitChild(Node* x, int i) void
        -insertNonFull(Node* x, T k) void
        -inorder(Node* x, function visit) void
        -destroy(Node* x) void
    }

    class Node {
        +vector~T~ keys
        +vector~Node*~ children
        +bool leaf
        +Node(bool isLeaf)
    }

    class Doctor {
        +string name
        +int age
        +string dob
        +string skill
        +string shiftStart
        +string shiftEnd
        +int skillLevel
        +visitMinutes() int
        +onShift(int minutesSinceMidnight) bool
        +fromCsv(string line, Doctor out)$ bool
        +toMinutes(string time12)$ int
        +operator<(Doctor o) bool
        +operator==(Doctor o) bool
    }

    class Patient {
        +string name
        +int age
        +string birthday
        +int condition
        +long arrival
        +int arrivalTime
        +critical() bool
        +fromCSV(string line, Patient out)$ bool
        +operator<(Patient o) bool
        +operator==(Patient o) bool
    }

    class PatientQueue {
        -Heap~Patient~ heap_
        -long nextArrival_
        +loadCSV(string path) LoadResult
        +admit(Patient p) void
        +peekNext() optional~Patient~
        +takeNext() optional~Patient~
        +worsenAll(int amount, int worst) void
        +waiting() vector~Patient~
        +count() size_t
    }

    class Heap~T~ {
        -vector~T~ data_
        -Before before_
        +push(T x) void
        +top() T
        +pop() T
        +updateAll(function change) void
        +rebuild() void
        +items() vector~T~
        +empty() bool
        +size() size_t
        +isValid() bool
        -siftUp(size_t i) void
        -siftDown(size_t i) void
    }

    class TreatFirst {
        +operator()(Patient a, Patient b) bool
    }

    class LoadResult {
        +int loaded
        +int skipped
        +int firstBadLine
        +string error
        +ok() bool
    }

    class HospitalView {
        -RenderWindow& window
        -Font& font
        +HospitalView(RenderWindow window, Font font)
        +draw(DisplayState state) void
        -drawClock(int time) void
        -drawWaiting(vector~string~ waiting) void
        -drawBeingSeen(vector~Visit~ beingSeen) void
        -drawText(string str, Vector2f pos, uint size) void
    }

    class DisplayState {
        +int time
        +vector~string~ waiting
        +vector~Visit~ beingSeen
    }

    class Visit {
        +string patient
        +string doctor
    }

    %% ownership (composition)
    Hospital "1" *-- "1" DoctorDB : doctors_
    Hospital "1" *-- "1" BTree : patients_ (T = Patient)
    Hospital "1" *-- "1" PatientQueue : waiting_
    Hospital "1" *-- "0..*" Treatment : visits_
    DoctorDB "1" *-- "1" BTree : tree_ (T = Doctor)
    BTree "1" *-- "0..*" Node : nodes
    PatientQueue "1" *-- "1" Heap : heap_ (T = Patient, Before = TreatFirst)
    DisplayState "1" *-- "0..*" Visit : beingSeen

    %% a visit pairs one patient with one doctor
    Treatment --> Patient
    Treatment --> Doctor

    %% uses / creates
    Heap ..> TreatFirst : orders by
    TreatFirst ..> Patient : compares
    Hospital ..> DisplayState : creates
    HospitalView ..> DisplayState : draws
    DoctorDB ..> LoadResult : returns
    Hospital ..> LoadResult : returns
    PatientQueue ..> LoadResult : returns
```

## How to read it

| Arrow | Meaning |
|---|---|
| `*--` (filled diamond) | Composition: the class owns the other and it lives inside it |
| `-->` | Association: holds a copy of / refers to |
| `..>` (dashed) | Dependency: uses, creates or returns |
| `+` / `-` | public / private |
| underlined (`$`) | static |

`BTree<T>` is one template used twice: `BTree<Patient>` inside `Hospital` and
`BTree<Doctor>` inside `DoctorDB`, both with minimum degree t = 3. `Heap<T, Before>`
is used as `Heap<Patient, TreatFirst>`, so the most urgent patient is always on top.
`main()` creates one `Hospital` and one `HospitalView`, calls `Hospital::step(5)` on
each Space press and draws `Hospital::display()` every frame.
