# Modular Source Architecture

## Source tree
```text
src/
  main.cpp                    Application bootstrap only
  database.cpp                SQLite schema, seed data, password hashing
  login_dialog.cpp            Login UI and credential verification
  main_window.cpp             Shared window shell, table/page helpers
  ui/
    admin_dashboard.cpp       Admin CRUD, filtering/search, CSV export
    lecturer_dashboard.cpp    Assignment creation, extensions, marking
    hod_dashboard.cpp         Moderation and mark release
    student_dashboard.cpp     Student submission and released results
include/lms/
  common.hpp                  Shared database/security function declarations
  domain.hpp                  Role model, factory and late penalty strategy
  ui/
    login_dialog.hpp          Login dialog interface
    main_window.hpp           Main window interface
tests/
  domain_tests.cpp
  policy_tests.cpp
```

## Responsibilities and dependencies
- `main.cpp` initialises QApplication, database, login, then opens MainWindow.
- `database.cpp` owns schema creation, demo seed records, password derivation and verification utilities.
- `login_dialog.cpp` owns the login form and calls shared verification functions.
- `main_window.cpp` owns common shell and shared UI helper implementations.
- Each role dashboard is compiled separately and defines one MainWindow role-workspace method.
- `domain.hpp` holds UI-independent role capabilities and late penalty policy.

## Build
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev libssl-dev libgtest-dev
cmake -S . -B build
cmake --build build -j2
ctest --test-dir build --output-on-failure
```

If `sudo` reports the `no new privileges` error, run these commands in a normal host terminal with administrative privileges, not a restricted container/sandbox.

## Debugging
Use the source file that owns the affected feature. For example, admin edit/filter/export issues are in `src/ui/admin_dashboard.cpp`; login issues are in `src/login_dialog.cpp`; database/schema issues are in `src/database.cpp`. Keep declarations in the matching header and add new `.cpp` files to `add_executable` in `CMakeLists.txt`.
