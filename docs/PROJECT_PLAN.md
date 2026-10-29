# Project Management Plan

## 1. Project definition
**Project:** Moderated Learning Management System (LMS)  
**Course:** CS4433 Software Engineering — Individual Assignment  
**Client:** Secondary school (case-study stakeholder)  
**Delivery:** C++17 desktop application, Qt Widgets GUI, SQLite local persistence  
**Development approach:** Waterfall, with V-Model verification and validation  
**Release target:** v1.0 followed by maintenance release v1.1 for CR1

The system supports four roles: Administrator, Lecturer, Head of Department (HoD), and Student. It runs offline on a school computer or local network. Online payments, video conferencing, automated grading, plagiarism detection, and web/mobile interfaces are out of scope.

## 2. Feasibility study
| Dimension | Assessment | Evidence / constraint | Decision |
|---|---|---|---|
| Technical | Feasible with qualification | C++17, Qt Widgets, SQLite and OpenSSL are mature; packaging and Qt availability must be verified on the target OS. | Proceed with a small proof-of-build and database prototype before full implementation. |
| Operational | Feasible | Role-specific tabs and guided forms fit the need for non-technical teachers. Offline operation avoids dependence on internet connectivity. | Provide short role-based manuals and acceptance demonstrations. |
| Economic | Likely feasible | The stack is open source; costs are mainly development time, staff training, backups and maintenance. | Use existing school hardware, subject to a performance test. |
| Legal | Feasible if controlled | Learner records and marks are personal/confidential data. Applicable Lesotho data-protection and education policies must be confirmed by the school. | Restrict access, minimise data, protect backups, define retention and correction procedures. |
| Schedule | Feasible but tight | Seven weeks must cover requirements, design, implementation, testing, deployment and maintenance. | Freeze scope at Gate 1; prioritise Must requirements; maintain a risk register. |

**Recommendation:** Proceed as an offline desktop prototype. Do not deploy to live learner records until security review, backup/restore, accessibility review, performance testing and institutional legal approval are completed.

## 3. Work breakdown structure
1. Project initiation and management
   1.1 Define scope and success criteria
   1.2 Feasibility and stakeholder analysis
   1.3 Schedule and risk register
2. Requirements engineering
   2.1 Resolve stakeholder conflicts
   2.2 Produce SRS and acceptance criteria
   2.3 Use-case model and specifications
   2.4 Gate 1 review
3. System design
   3.1 Layered architecture and component diagram
   3.2 Class, state, activity and sequence diagrams
   3.3 Normalised database schema and data dictionary
   3.4 Design review and Gate 2
4. Implementation
   4.1 Authentication and role-specific GUI
   4.2 Course, enrolment and assignment workflows
   4.3 Submission, extensions and late-penalty policy
   4.4 Marking, moderation, release and audit
   4.5 Persistence and error handling
5. Verification and validation
   5.1 Unit tests and boundary-value tests
   5.2 Integration tests for SQLite repositories
   5.3 End-to-end role-based system tests
   5.4 Acceptance tests and defect log
   5.5 Gate 3 review
6. Configuration management and quality assurance
   6.1 Git repository and branch policy
   6.2 CI build/test workflow
   6.3 Static analysis and code review
   6.4 Coverage and metrics
7. Deployment
   7.1 Package release v1.0
   7.2 Installation and role manuals
   7.3 Deployment diagram and release notes
8. Maintenance
   8.1 Assess CR1
   8.2 Implement class-wide extensions
   8.3 Regression tests and release v1.1
9. Completion
   9.1 Professional/legal/ethical review
   9.2 Traceability and final report
   9.3 Viva preparation

## 4. Seven-week schedule
| Work package | W1 | W2 | W3 | W4 | W5 | W6 | W7 | Dependency / milestone |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| Planning and feasibility | X | | | | | | | Start; baseline scope |
| Requirements and SRS | X | X | | | | | | Planning; Gate 1 at end W2 |
| Architecture/UML/database | | X | X | | | | | Requirements; Gate 2 at end W3 |
| Core implementation | | | | X | X | | | Gate 2 approval |
| Unit/integration testing | | | | | X | X | | Core implementation increments |
| System/acceptance testing | | | | | | X | | Stable build; Gate 3 |
| CI/static analysis | | | X | X | X | X | | Repository created early |
| Deployment/user manuals | | | | | | X | | Gate 3 |
| CR1 implementation/regression | | | | | | | X | v1.0 baseline |
| Final report/viva | | | | | | | X | All artefacts |

**Critical path:** requirements baseline → design approval → core workflows → integration/system tests → release v1.0 → CR1 regression → final report. Parallel work on CI, documentation and test design reduces schedule risk. The table is a planning baseline; actual start/finish dates and completion evidence must be recorded by the student.

## 5. Risk register
Scale: likelihood (L) and impact (I) each 1–5; exposure = L × I.
| ID | Risk | L | I | Exposure | Mitigation / contingency | Owner |
|---|---|---:|---:|---:|---|---|
| R1 | Qt/OpenSSL dependencies unavailable on target machine | 3 | 4 | 12 | Build early on clean Kali/Ubuntu; document package versions; retain compiler logs. | Developer |
| R2 | Scope exceeds seven-week schedule | 4 | 5 | 20 | Freeze scope at Gate 1; prioritise Must requirements; track weekly WBS. | Developer |
| R3 | Incorrect late-penalty interpretation | 3 | 5 | 15 | Define rule with HoD; test day 0–4 boundaries; state whether deduction is from maximum mark. | Developer/HoD |
| R4 | Unauthorised access to marks | 3 | 5 | 15 | Role checks at workflow boundary, least privilege, negative tests, secure password hashes. | Developer |
| R5 | Database corruption or loss on a single PC | 3 | 5 | 15 | Scheduled offline backups, restore test, safe shutdown and documented recovery. | IT officer |
| R6 | Users misunderstand moderation/release state | 3 | 4 | 12 | Explicit statuses, clear labels, role manuals and acceptance testing. | Developer/teachers |
| R7 | Incomplete audit trail for mark changes | 3 | 5 | 15 | Record actor/time/old/new/reason in a transaction; test every mark update path. | Developer |
| R8 | Deadline/time-zone parsing creates incorrect lateness | 3 | 4 | 12 | Use a consistent local timestamp convention and test boundary timestamps. | Developer |
| R9 | Student resubmission overwrites evidence needed for review | 2 | 4 | 8 | Consider versioned submission records in next iteration; disclose current behaviour. | Developer |
| R10 | Accessibility barriers for keyboard/low-vision users | 3 | 3 | 9 | Keyboard traversal, labels, contrast, scalable fonts and user review. | Developer |
| R11 | Demo credentials are not changed before real use | 3 | 5 | 15 | Force password change in a hardened release; never load demo accounts into production. | Administrator |
| R12 | CI/test evidence is claimed without actual execution | 3 | 4 | 12 | Archive real logs and coverage reports; mark unrun checks as pending, never fabricate results. | Developer |

Review the register at Gate 1 (requirements/scope), Gate 2 (design/security/feasibility), Gate 3 (test findings/release readiness), and during CR1. Update likelihood, impact, owner and mitigation based on evidence.
