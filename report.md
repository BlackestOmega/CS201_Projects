# Project 3 Report

## 1. How does your simulation work? Provide an example, describe the visual component.

Patients are loaded into the system from a CSV, and are placed into a max-heap. The heap is ordered according to TreatFirst, which ranks patients by condition, then arrival time. The most urgent patient can be peeked with peekNext, and they can be removed with takeNext, which returns the patient. The contents of the private heap can be handed off as a vector to the GUI with waiting(). 

Doctors are also loaded from a CSV, and they are placed into a B-tree. 

## 2. For your B-Tree, what value of t was chosen, and why?

The DoctorDB class defines a minimum degree of 3, so that B-tree properties are preserved.

## 3. What was each team members contribution?

Joseph Mire integrated the B trees, csv, and GUI. Joseph also re-organized much of the code. 
Matthew Carter implemented the B tree data algorithm for the Doctor information and generated the patient and doctor information that we are using.
Sam Bozeman made the UI for the doctors and patients.
Daniel Healy wrote code for the patients and compiled the report.