#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "signal_tests.h"
#include "thread_tests.h"

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */

int main()
{
    CU_pSuite thread_suite_p = NULL;
    CU_pSuite signal_suite_p = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
    {
        return CU_get_error();
    }

    /* add a suite to the registry */
    thread_suite_p = CU_add_suite("Thread Suite", NULL, NULL);
    if (NULL == thread_suite_p)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    signal_suite_p = CU_add_suite("Signal Handling Suite", NULL, NULL);
    if (NULL == signal_suite_p)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(thread_suite_p,
                             "test of threadpool_create()",
                             test_threadpool_create)) ||
        (NULL == CU_add_test(thread_suite_p,
                             "test of add null pool",
                             test_threadpool_add_job_null_pool)) ||
        (NULL == CU_add_test(thread_suite_p,
                             "test of add null job",
                             test_threadpool_add_null_job)) ||
        (NULL == CU_add_test(thread_suite_p,
                             "test of threadpool_add_job()",
                             test_threadpool_add_job)) ||
        (NULL == CU_add_test(thread_suite_p,
                             "test of threadpool limit",
                             test_threadpool_limit)) ||
        (NULL == CU_add_test(thread_suite_p,
                             "test of threadpool shutdown()",
                             test_threadpool_shutdown)) ||
        (NULL == CU_add_test(signal_suite_p,
                             "test of signal handler()",
                             test_signal_handler)) ||
        0)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    unsigned int ret_state = CU_get_number_of_tests_failed();
    CU_cleanup_registry();
    return ret_state; // NOLINT
}
