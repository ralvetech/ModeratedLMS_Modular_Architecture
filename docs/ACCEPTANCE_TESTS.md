# Acceptance Test Checklist

Run each test on a fresh database and record actual results before submission.

| ID | Role | Test | Expected result |
|---|---|---|---|
| AT-01 | All | Sign in with valid account | Role-specific workspace opens |
| AT-02 | All | Sign in with incorrect password | Access denied without exposing stored password |
| AT-03 | Administrator | Create user with unique username | User appears in account table |
| AT-04 | Administrator | Create duplicate username | SQLite constraint prevents duplicate |
| AT-05 | Administrator | Create course and assign lecturer | Course is linked to selected lecturer |
| AT-06 | Administrator | Enrol student | Student sees course assignments |
| AT-07 | Lecturer | Create assignment | Assignment appears for enrolled students |
| AT-08 | Student | Submit before deadline | Status is Submitted |
| AT-09 | Student | Submit after deadline | Work is accepted with Late status |
| AT-10 | Student | Resubmit before release | Latest content and submission timestamp are stored |
| AT-11 | Lecturer | Mark submission | Raw/final mark and feedback are saved |
| AT-12 | Lecturer | Mark a late submission | Late penalty is applied to final mark |
| AT-13 | HoD | Moderate a marked submission | Status becomes Moderated |
| AT-14 | HoD | Release moderated mark | Status becomes Released and student can see mark/feedback |
| AT-15 | Student | Inspect unreleased mark | No mark or feedback is displayed |
| AT-16 | Lecturer | Grant class extension | Updated deadline appears in student assignment list |
| AT-17 | Lecturer | Change mark | Mark audit captures actor, timestamp, old/new mark and reason |
| AT-18 | Any | Restart application | SQLite data persists |
