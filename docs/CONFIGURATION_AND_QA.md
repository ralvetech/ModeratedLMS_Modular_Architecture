# Configuration Management and Quality Assurance

## Git workflow
- `main`: protected release-ready branch.
- `feature/<short-name>`: one requirement/change per branch.
- `fix/<defect-id>-<short-name>`: bug fixes.
- Open a pull request, run CI, review code and merge only when build/tests pass.
- Use conventional commit subjects, e.g. `feat: add class-wide deadline extension`.
- Tag reviewed release commits `v1.0.0` and `v1.1.0`.
- Do not commit database files, passwords, personal student data, build output or test reports containing personal information.

## Continuous integration
`.github/workflows/ci.yml` installs compiler/CMake/Qt/OpenSSL, configures, builds and runs CTest on pushes and pull requests. A green workflow must be observed in the hosted repository before claiming CI success. A GoogleTest suite in `tests/domain_tests.cpp` covers the production late-penalty strategy and role capability model. The CMake configuration uses it when GoogleTest is installed and falls back to the standalone policy test otherwise. Run the CI workflow and retain its actual output as evidence.

## Static analysis and metrics
Run:
```bash
sudo apt install cppcheck
cppcheck --enable=warning,style,performance,portability --std=c++17 src tests
```
Optional compiler diagnostics:
```bash
cmake -S . -B build -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"
cmake --build build --clean-first -j2
```
Record the actual output, warnings resolved, source LOC and complexity metrics. Do not claim zero warnings or 70% coverage until measured. Consider `clang-tidy` if installed and configured.

## Code review checklist
- [ ] All SQL values are parameterised.
- [ ] Workflow authorization is enforced in services, not only by hiding buttons.
- [ ] Passwords are never stored or logged in plaintext.
- [ ] Mark update and audit insert occur in one transaction.
- [ ] HoD access is limited to their own department.
- [ ] Students cannot see marks until released.
- [ ] Deadline/extension precedence is consistent across display and penalty calculation.
- [ ] Invalid input and DB errors are handled.
- [ ] Tests cover boundary and exception paths.
- [ ] User-visible messages are accessible and understandable.
- [ ] Backups and restoration have been tested.


## Current code hardening status
The late-penalty calculation is now implemented as `StandardLatePenalty`; mark and audit persistence are transactional; and the HoD audit view is department-scoped. The abstract role hierarchy and factory are present in `include/lms/domain.hpp`, but further refactoring is still needed to make every workflow service use this model as its sole authorization authority. Do not claim the whole application has passed a security review until this is completed and tested.
