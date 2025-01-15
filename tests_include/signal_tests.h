#ifndef SIGNAL_TESTS_H
#define SIGNAL_TESTS_H

/**
 * @brief Test a signal handler with CUnit
 * Test that SIGUSR1 signal is caught and the global signal number is set to 1
 * Test that SIGINT signal is caught and the global signal number is set to 2
 *
 * Calls signal_action_setup() in test
 */
void test_signal_handler(void);

#endif