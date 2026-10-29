# Verification and Validation Plan (V-Model)

## 1. Verification levels and entry/exit criteria
| Level | Verifies | Entry criteria | Exit criteria | Method |
|---|---|---|---|---|
| Unit | Late penalty, deadline resolution, input validation, state transition rules, password verification | Design reviewed; policy decisions approved | All unit tests pass; boundary partitions covered; no unresolved critical defects | GoogleTest/Catch2 preferred; current policy harness is a small CTest executable and must be upgraded to the mandated framework before final submission |
| Integration | GUI/workflow-to-SQLite, transaction and audit writes, role queries | Unit tests pass; schema migrations are stable | Data persists; constraints work; audit is atomic with mark changes | Temporary test database and SQL assertions |
| System | End-to-end four-role workflows, offline operation, error paths | Integrated build available | All Must requirement scenarios pass; no open severity-1/2 defects | Scripted manual/system tests |
| Acceptance | Principal, lecturer, HoD, student and IT expectations | System tests pass; manuals available | Stakeholders approve agreed acceptance criteria | Demonstration and signed acceptance record |

## 2. Test design techniques
- Equivalence partitioning: active vs inactive user; correct vs incorrect password; authorised vs unauthorised role; enrolled vs non-enrolled student; marked vs unmarked vs released submission.
- Boundary-value analysis: raw mark 0, maximum, negative and above maximum; due time one second before, exactly at and one second after deadline; late days 0, 1, 3 and 4; password length 9 vs 10; 800 student records performance target.
- State-transition testing: Submitted/Late → Marked → Moderated → Released; reject release before moderation; reject student edits after release.
- Negative testing: duplicate username/course code; empty submission; invalid foreign key; DB read-only/full; missing audit reason; cross-department HoD access.

## 3. Automated policy test
`tests/policy_tests.cpp` currently tests eight penalty boundary cases through CTest. It is a starter test harness, **not evidence of 70% business-logic coverage** and not a substitute for the required GoogleTest or Catch2 suite. Integrate the policy into a production service function and test that exact function to avoid duplicating business logic.

## 4. System acceptance tests
See `ACCEPTANCE_TESTS.md`. Run every test on a clean database and record:
- build commit and OS/dependency versions;
- input fixture and role used;
- expected result and actual result;
- pass/fail;
- defect ID and retest result.

## 5. Defect log template
| Defect ID | Severity | Reproduction | Cause | Fix/commit | Retest result | Status |
|---|---|---|---|---|---|---|
| D-001 | To assign | To record after execution | To investigate | To record | Not run | Open |

Do not invent test results. Fill the defect log only after executing tests on the target environment.
