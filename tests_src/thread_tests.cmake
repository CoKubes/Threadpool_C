cmake_minimum_required(VERSION 3.16)

# Create the tests binary if build.sh test is run
include_directories(tests_include)

# Create the tests binary
add_executable(tests_threads
    ${CMAKE_CURRENT_LIST_DIR}/signal_tests.c
    ${CMAKE_CURRENT_LIST_DIR}/thread_tests_main.c
    ${CMAKE_CURRENT_LIST_DIR}/thread_tests.c
)
target_link_libraries(tests_threads threadpool cunit pthread)
INSTALL(TARGETS tests_threads DESTINATION ${PROJECT_SOURCE_DIR}/bin)

add_test(
    NAME TestThreadCalc
    COMMAND python3 ${CMAKE_SOURCE_DIR}/Tests/ThreadCalcTests.py
)

add_test(
    NAME TestThreadLib_valgrind
    COMMAND valgrind --leak-check=yes --show-reachable=yes --error-exitcode=1 $<TARGET_FILE:tests_threads>
)

add_test(
    NAME TestThreadLib_helgrind
    COMMAND valgrind --tool=helgrind --error-exitcode=1 $<TARGET_FILE:tests_threads>
)
