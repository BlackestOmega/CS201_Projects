# Project 3 Report

## 1. How does your simulation work? Provide an example, describe the visual component.

### Patient

Patients are loaded into the system from a CSV, and are placed into a max-heap (PatientQueue) and a B-tree. The heap is ordered according to TreatFirst, which ranks patients by condition, then arrival time. The most urgent patient can be peeked with peekNext(), and they can be removed with takeNext(), which returns the patient. The contents of the private heap can be handed off as a vector to the GUI with waiting(). The B-tree acts as a central database for all the patients, ensuring that they are accounted for even when outside of the heap, and it works similarly to the doctors' B-tree described below, sorting by name, then age, then date of birth.

As an example, let us consider Robert Mccann, age 59 and date of birth 1967-04-11. The optional two categories of his CSV entry are filled out, so he enters the hospital at 8:00 AM with a condition severity of 6. Hospital's loadPatients() method scans his line from the CSV, then sends it to the Patient method fromCSV() to extract the data and set up the Patient. He is also added to the patients_ B-tree in loadPatients(). At the end of loadPatients(), admitArrivals() is called. Once he has "arrived to the hospital," denoted by his arrival time of 8 AM, this method adds him to the _waiting priority queue. If he has high enough condition severity, the method assignDoctors() is then called and acts on him, creating a Treatment that takes an amount of time based on the doctor's skill level, discussed below. Further, this method removes him from the priority queue. The Hospital method finishVisits() releases him from the hospital after his visit ends, though his records are still kept in the B-tree for furute reference.

### Doctor

Doctors, like Patients, are also loaded from a CSV, and they are placed into a B-tree. They are sorted within the tree by operator overloads to < and ==, which compare name, then age, then date of birth. Doctors can be searched with just their name, or they can be searched with name, age, and date of birth.

For an example, let us take Whitney Hill from the doctors CSV. Hospital's loadDoctors() method is the first stop on her journey, which adds her to the DoctorDB doctors_. The DoctorDB method loadCSV() adds her and her information (name, age, date of birth, medical specialization, hours of work, and skill level) to a Doctor struct and places said struct into the internal B-tree of doctors_. Initially, she isn't in a visit with anyone, so Hospital's method finishVisits() does nothing on the first pass. The method assignDoctors() will give her a patient and set up a Treatment while she is on call, which is determined by the Doctor method onShift(), comparing her shift's start and end times to the current time. Since she has a skill level of 2, a visit takes 60 - (10*2) = 40 minutes. While she is in a visit with a patient, further calls of assignDoctors() will skip over her, as the check using the Hospital method busy() will show she is occupied. Once the Treatment reaches its end time, the Hospital method finishVisits() will remove the completed Treatment from the list of those ongoing, and Whitney will no longer be marked occupied when busy() is called, freeing her up to take on another Patient.

### Hospital

The Hospital makes use of all the data structures mentioned above to do high-level command and control. It keeps track of the time since the beginning of the first day of the simulation, and the driver program sets this time such that it processes arrivals beginning at 8 AM. A press of the space bar advances the time of day by five minutes, executed by step(). This method finishes visits that have reached their ends, worsens the conditions of the patients still in the waiting_ priority queue by 1 with a call to worsenAll(), admits new arrivals to the hospital, and finally sets up Treatments between Patients and Doctors. These Treatments are stored in the visits_ vector. The end of a visit, handled by finishVisits(), removes the Treatments that ended from visits_, which frees up the doctor to take on a new patient and increments the number of patients that have been discharged.

### Visuals

The HospitalView class is used to draw the dashboard for the hospital, implemented using SFML. The public method draw() uses private methods to draw the clock, the patients waiting to be seen, and those being seen at the current moment. The time is given at the top-left corner, the numbered waiting list is placed on the left side of the window, and the list of ongoing visits is placed on the right side of the window.

## 2. For your B-Tree, what value of t was chosen, and why?

The DoctorDB class defines a minimum degree of 3 (B=2), so that B-tree properties are preserved. In the cases being used here, with a limited number of doctors, this value is reasonable. For a larger hospital than what is depicted in the doctors' and patients' CSVs, or an entire hospital network, a larger degree (and thus value of B) may be warranted to take advantage of the B-tree's minimization of disk reads when compared to other tree structures.

## 3. What was each team member's contribution?

Joseph Mire integrated the B-trees, CSV, and GUI. Joseph also re-organized much of the code. Matthew Carter implemented the B tree data algorithm for the Doctor information and generated the patient and doctor information that we are using.
Sam Bozeman made the UI for the doctors and patients.
Daniel Healy wrote backend code for the patients and compiled the report.