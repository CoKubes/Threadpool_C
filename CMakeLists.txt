cmake_minimum_required(VERSION 3.16)

set(NAME "threadcalc")
project(${NAME}
        LANGUAGES "C")

# BUILD THE EXECUTABLE BINARY IN THE 4_ThreadCalc/bin DIRECTORY.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#set CMAKE flags
set(CMAKE_C_FLAGS "-g -Wall -pedantic")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include ../0_Common/include ../3_DataStructures1/include)

# CREATE A LIBRARY OF YOUR THREADPOOL.
add_library(threadpool 
    ./src/threadpool.c
    ./src/signals.c
    ../3_DataStructures1/src/queue.c)

# Specify include directories for the threadpool library
target_include_directories(threadpool PUBLIC ${PROJECT_SOURCE_DIR}/include ../3_DataStructures1/include)

#Find common library
find_library(COMMON_LIB NAMES libcommon.a PATHS ../0_Common/build/)
if (NOT COMMON_LIB)
    message(FATAL_ERROR "Library libarithmetic.a not found")
endif ()

# Create an executable for your threadcalc main function
add_executable(threadcalc ./src/threadcalc_main.c)

# Specify include directories for the threadcalc executable
target_include_directories(threadcalc PUBLIC ${PROJECT_SOURCE_DIR}/include ../0_Common/include ../3_DataStructures1/include)

# Link the threadpool library and the common library to the executable
target_link_libraries(threadcalc threadpool ${COMMON_LIB} pthread)

# THIS INCLUDES THE TESTS FOR ThreadCalc. DO NOT MODIFY!
include(tests_src/thread_tests.cmake)
