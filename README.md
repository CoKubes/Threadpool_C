Threadpool Library & Tests

## Purpose
These tests are designed to test a general threadpool implementation  as a stand-alone threadpool library.

## Building
### Dependencies
- C11 or higher, C17 preferred
- Clang-Tidy
- Valgrind
- CMake 3.16 or higher
- CUnit

## Testing
The Threadpool tests check the functionality of critical threadpool functions and tasks, memory management and thread safety, and the memory management and thread safety of the standalone Threadpoolc executable. 

### Signal Handling
The test suite also provides checks for a signal handler, which is not required to be integrated into the threadpool directly for testing purposes (refer to your project spec on implementation requirements), but can be if desired. If signal handling tests are not desired or implemented, modify the tests appropriately. Be aware, if signal handling is not implemented, a uncaught signal **will stop** the tests from proceeding unless modified.

### Running the Tests
The tests are integrated into the Gitlab CI/CD for your repo, and run automatically when a commit is pushed to the repo. The tests can also be run locally from the build folder with `ctest`. This will run all tests built in the repo, including other projects:
```bash
user@user:~/CalculatorProjects/build$ ctest -V
...
...
...
