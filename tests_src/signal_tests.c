// NOLINTNEXTLINE
#define _XOPEN_SOURCE 700 // NO LINT prevent incomplete type "struct sigaction"

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "signals.h"
#include "utils.h"
#include "signal_tests.h"

void test_signal_handler(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(signal_handler);

    /*Check the starting value of signal_flag_g*/
    CU_ASSERT_EQUAL(0, signal_flag_g);

    /*Assert that the signal action setup was run and returned success*/
    CU_ASSERT_EQUAL_FATAL(signal_action_setup(), SUCCESS_);

    pid_t procID = getpid();
    /*Raise Signal SIGUSR1 and signal_flag_g should equal 1*/
    kill(procID, SIGUSR1);
    CU_ASSERT_EQUAL(1, signal_flag_g);

    /*Raise Signal SIGINT and signal_flag_g should equal 2*/
    kill(procID, SIGINT);
    CU_ASSERT_EQUAL(2, signal_flag_g);
}
