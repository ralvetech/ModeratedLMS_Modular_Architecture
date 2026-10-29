cmake_minimum_required(VERSION 3.20)
project(ModeratedLMS VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
find_package(OpenSSL REQUIRED)
add_executable(moderated_lms
    src/main.cpp
    src/database.cpp
    src/login_dialog.cpp
    src/main_window.cpp
    src/ui/admin_dashboard.cpp
    src/ui/lecturer_dashboard.cpp
    src/ui/hod_dashboard.cpp
    src/ui/student_dashboard.cpp
)
target_link_libraries(moderated_lms PRIVATE Qt6::Widgets Qt6::Sql OpenSSL::Crypto)
target_include_directories(moderated_lms PRIVATE include)
target_include_directories(moderated_lms PRIVATE ${OPENSSL_INCLUDE_DIR})

include(CTest)
if(BUILD_TESTING)
  find_package(GTest QUIET)
  if(GTest_FOUND)
    add_executable(domain_tests tests/domain_tests.cpp)
    target_include_directories(domain_tests PRIVATE include)
    target_link_libraries(domain_tests PRIVATE GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(domain_tests)
  else()
    message(STATUS "GoogleTest not found; running fallback policy tests only")
    add_executable(policy_tests tests/policy_tests.cpp)
    add_test(NAME policy_tests COMMAND policy_tests)
  endif()
endif()
