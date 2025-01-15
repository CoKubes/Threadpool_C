# ThreadCalc & Threadpool Library Tests

## Purpose
These tests are designed to test a general threadpool implementation either as part of the ThreadCalc project, or as a stand-alone threadpool library. As such, these tests do not check for integration with the rest of the Calculator Projects or some of the ThreadCalc tasks. 

## Building
### Dependencies
- C11 or higher, C17 preferred
- Clang-Tidy
- Valgrind
- CMake 3.16 or higher
- CUnit

```bash
sudo apt install clang-tidy valgrind cmake libcunit1 libcunit1-doc libcunit1-dev
```
Build the ThreadCalc executable and tests with the `build.sh` script from the root directory:

```zsh
user@user:~/CalculatorProjects$ bash build.sh
-- The C compiler identification is GNU 9.4.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
...
...
...
[100%] Linking C executable ../../1_SimpleCalc/bin/tests_simplecalc
[100%] Built target tests_threads
[100%] Built target tests_simplecalc
```


## Testing
The ThreadCalc tests check the functionality of critical threadpool functions and tasks, memory management and thread safety, and the memory management and thread safety of the standalone ThreadCalc executable. If these tests are being used for a standalone threadpool, modify the `4_ThreadCalc/tests_src/thread_tests.cmake`, `4_ThreadCalc/CMakeLists.txt` and root `CMakeLists.txt` as needed to reflect the lack of a ThreadCalc project.

### Signal Handling
The test suite also provides checks for a signal handler, which is not required to be integrated into the threadpool directly for testing purposes (refer to your project spec on implementation requirements), but can be if desired. If signal handling tests are not desired or implemented, modify the tests appropriately. Be aware, if signal handling is not implemented, a uncaught signal **will stop** the tests from proceeding unless modified.

### Running the Tests
The tests are integrated into the Gitlab CI/CD for your repo, and run automatically when a commit is pushed to the repo. The tests can also be run locally from the build folder with `ctest`. This will run all tests built in the repo, including other projects:
```bash
user@user:~/CalculatorProjects/build$ ctest -V
...
...
...
     CUnit - A unit testing framework for C - Version 2.1-3
     http://cunit.sourceforge.net/


Suite: Thread Suite
  Test: test of threadpool_create() ...passed
  Test: test of threadpool error code ...passed
  Test: test of add null pool ...passed
  Test: test of add null job ...passed
  Test: test of threadpool_add_job() ...passed
  Test: test of threadpool limit ...passed
  Test: test of threadpool shutdown() ...passed
Suite: Signal Handling Suite
  Test: test of signal setup ...passed
  Test: test of signal handler() ...passed

Run Summary:    Type  Total    Ran Passed Failed Inactive
              suites      2      2    n/a      0        0
               tests      9      9      9      0        0
             asserts     84     84     84      0      n/a

Elapsed time =    0.011 seconds
```
To run only the ThreadCalc tests from the root directory (python for functionality, valgrind and helgrind on the library tests):
```
cd build && ctest -R TestThreadCalc
```
```
cd build && ctest -R TestThreadCalcLib_valgrind
```
```
cd build && ctest -R TestThreadCalcLib_helgrind
```


The test executable can also be run without valgrind and helgind:
```bash
user@user:~/CalculatorProjects$ ./4_ThreadCalc/bin/tests_thread
```

## Note on Atomics
While C11 Atomics are thread safe, due to the instrumentation of the helgrind tool, they may show as a potential race condition in error output. There are a variety of techniques to address and suppress this output, but ultimately the trainee and reviewer should conduct a thorough review to ensure reported potential race conditions are not actually present.

## Additional Testing and Debugging Tools
These tests are conducted with valgrind, using the memcheck and helgrind tools. The valgrind DRD (Data Race Detector) can also be used for multithreaded program analysis. It tends to report fewer errors on atomics, but can have less clear error output. Additionally, the thread-sanitizer tool (a similar tool to Address Sanitizer) provides good debugging of data races.

## Other Test Errors
This test suite is nearly free from warnings, errors, clang-tidy output, memory leaks and thread errors. If an error is observed and believed to be from this test suite, open an issue ticket on the main repo and provide enough information to reproduce the error.

### Future Improvements
In a future version of this library, all tested functions will be compiled as weak symbols. This will allow a trainee to run tests on an incomplete project and test throughout their development process. Currently, this project will not build without a definition for each declared tested function.

Further questions can be directed to 1LT Gutzmann.