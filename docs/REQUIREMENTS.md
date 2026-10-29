# Requirements Engineering

## 1. Stakeholders and conflict resolution
| Stakeholder | Primary interests | Resolution / agreed rule |
|---|---|---|
| Principal | Simple operation; no marks visible before HoD check | Marks and feedback remain hidden until status `Released`. |
| Lecturer | Prompt marking; extensions for legitimate circumstances | Lecturer can mark and provide feedback, but marks remain provisional until HoD moderation/release. Individual and class-wide extensions are supported. |
| HoD | Department-wide review; consistent late policy | HoD can review all submissions for courses in their department. Late work is accepted up to three days; deduction is 10% of the maximum available mark per late day; after three days, zero. This interpretation must be approved by the school because the brief says “10% of the available mark.” |
| Student representative | Resubmit before deadline; written feedback; late work not rejected | Student may resubmit before the effective deadline; late submissions are accepted and flagged. Feedback is required for meaningful marking and visible only after release. |
| IT officer | Offline, protected passwords, quick response for 800 students | Local SQLite storage, password hashing and indexed lookups. “800 students” is a test target, not a proven performance claim until measured. |

## 2. Software Requirements Specification (SRS)
Priority: M = Must, S = Should, C = Could. Requirements are written to be verifiable.

### 2.1 Functional requirements
| ID | Requirement | Priority | Verification |
|---|---|---|---|
| FR-01 | The system shall authenticate an active user using a unique username and password. | M | Login test with valid/invalid credentials |
| FR-02 | The system shall provide role-specific capabilities for Administrator, Lecturer, HoD and Student. | M | Positive and negative role tests |
| FR-03 | The Administrator shall create, activate/deactivate and maintain user accounts. | M | Account CRUD acceptance test |
| FR-04 | The Administrator shall create departments and courses. | M | Database and GUI test |
| FR-05 | The Administrator shall assign lecturers to courses. | M | Assignment test |
| FR-06 | The Administrator shall enrol students in courses. | M | Enrolment test |
| FR-07 | A Lecturer shall create an assignment with title, description, deadline and positive maximum mark. | M | Boundary validation test |
| FR-08 | A Lecturer shall grant an individual student an extension and record a reason. | M | Extension workflow test |
| FR-09 | A Lecturer shall grant a class-wide extension; the effective deadline shall appear in each enrolled student's assignment list. | M | CR1 regression test |
| FR-10 | A Student shall view enrolled courses, assignment details and effective deadlines. | M | Student acceptance test |
| FR-11 | A Student shall submit and resubmit assignment work before release. | M | Submission/resubmission test |
| FR-12 | The system shall accept late work and calculate a penalty according to the approved rule. | M | Boundary tests for 0–4 late days |
| FR-13 | A Lecturer shall mark a submission and provide written feedback. | M | Marking test |
| FR-14 | The system shall prevent students from viewing marks and feedback before release. | M | Access-control test |
| FR-15 | A HoD shall review and moderate marks for courses in their department. | M | Department-scope test |
| FR-16 | A HoD shall release moderated marks and feedback to students. | M | Release lifecycle test |
| FR-17 | The system shall record each mark change with actor, timestamp, old value, new value and reason. | M | Audit-record assertion |
| FR-18 | The system shall retain data across application restarts. | M | Persistence test |
| FR-19 | The system shall validate user input and report database errors without an unhandled crash. | M | Invalid-input and DB-error tests |
| FR-20 | The system shall allow authorised users to view mark audit history. | S | Audit access test |

### 2.2 Non-functional requirements
| ID | Requirement | Priority | Verification |
|---|---|---|---|
| NFR-01 Performance | With 800 student accounts and representative course/assignment records on the target school computer, login shall complete within 2 seconds at the 95th percentile in a documented test of at least 100 attempts. | M | Timed performance test |
| NFR-02 Offline operation | Core login, course, assignment, submission, marking and moderation workflows shall work with network connectivity disabled. | M | Disconnect network and run acceptance suite |
| NFR-03 Password security | Passwords shall not be stored in plaintext; each account shall have a unique random salt and a recognised password KDF. | M | Schema/code inspection and test |
| NFR-04 Access control | All unauthorised role/workflow attempts in the negative-test suite shall be rejected. | M | Security test matrix |
| NFR-05 Data integrity | Foreign-key constraints shall reject invalid references; transactions shall preserve consistent mark/audit updates. | M | Database constraint and rollback tests |
| NFR-06 Usability | A representative teacher shall complete create-assignment and mark-submission tasks after no more than 15 minutes of orientation, with at least 90% task completion in a small usability test. | S | Observed usability test |
| NFR-07 Maintainability | Business rules shall be separated from GUI and persistence; build instructions shall permit a clean build on a documented supported OS. | M | Code review and clean build |
| NFR-08 Compatibility | The release shall build with a documented C++17 compiler, CMake, Qt 6 Widgets/Sql and OpenSSL version. | M | Clean environment build |
| NFR-09 Recovery | The administrator shall be able to back up the database and restore it to a test installation, with the restore procedure completed in 15 minutes or less. | S | Timed recovery drill |
| NFR-10 Auditability | Every successful mark update shall have exactly one matching audit event committed with it. | M | Transaction/integration test |

**Performance caveat:** NFR-01 is a target, not a measured result. Record hardware, dataset size, number of attempts, mean, p95 and failures before claiming compliance.

## 3. Fully dressed use cases

### UC-01 Submit assignment
**Primary actor:** Student  
**Preconditions:** Student is authenticated and active; student is enrolled in the course; assignment exists; submission has not been released.  
**Trigger:** Student selects an assignment and chooses Submit/Resubmit.  
**Main flow:**
1. System displays assignment details and effective deadline.
2. Student enters submission content.
3. System validates non-empty content and eligibility.
4. System compares submission time with the effective deadline.
5. System saves the submission and timestamp.
6. System sets status to `Submitted` or `Late`.
7. System confirms the result.
**Alternative flows:** A1: Existing unreleased submission is updated as a resubmission. A2: Submission is late but within the accepted submission window; accept and flag it. A3: A class or individual extension changes the effective deadline.  
**Exceptions:** E1: Student is not enrolled; reject. E2: Submission is empty; ask for correction. E3: Database write fails; show error and do not claim success. E4: Mark is released; reject changes.  
**Postconditions:** Latest submission is persisted with status and timestamp.  
**Business rules:** The effective deadline is individual extension, else class extension, else original deadline. Late work is accepted. Resubmission behaviour and whether previous versions must be retained should be approved by the school.

### UC-02 Mark submission
**Primary actor:** Lecturer  
**Preconditions:** Lecturer is authenticated and assigned to the submission's course; submission exists and is not released.  
**Trigger:** Lecturer selects Mark.  
**Main flow:** 1. System checks course ownership. 2. Lecturer enters raw mark, written feedback and reason. 3. System validates 0 ≤ raw mark ≤ maximum mark. 4. System calculates any late penalty. 5. System saves raw/final mark, feedback, marker and `Marked` status. 6. System records the audit event in the same transaction. 7. System confirms marking.  
**Alternative flows:** An extension means no penalty if submission is before the effective deadline. A resubmission resets the prior unreleased mark and must be marked again.  
**Exceptions:** Invalid mark, missing reason, unauthorised course, released record or database failure.  
**Postconditions:** Mark is stored and awaiting HoD moderation; it is not visible to the student.  
**Business rules:** The late deduction is 10% of maximum mark per late day for up to three days; more than three late days yields zero. Confirm whether a partial day counts as one full day with the school.

### UC-03 Moderate and release marks
**Primary actor:** Head of Department  
**Preconditions:** HoD is authenticated; submission belongs to a course in the HoD's department; lecturer has marked the submission.  
**Trigger:** HoD selects a marked submission.  
**Main flow:** 1. System scopes the queue to the HoD's department. 2. HoD reviews mark, feedback and audit context. 3. HoD records moderation outcome/reason. 4. System changes status to `Moderated`. 5. HoD explicitly releases the mark. 6. System stores release timestamp and status `Released`. 7. Student view displays mark and feedback.  
**Alternative flows:** HoD may request correction instead of releasing; the submission returns to the lecturer for amendment, preserving audit history.  
**Exceptions:** Unmarked submission, wrong department, repeated release or persistence error.  
**Postconditions:** Released mark is visible to the student; prior mark changes remain auditable.  
**Business rules:** A mark must not be visible before explicit release by the HoD.
