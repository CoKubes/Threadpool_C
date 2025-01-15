// NOLINTNEXTLINE
#define _XOPEN_SOURCE 700 // NO LINT nanosleep declaration

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "threadpool.h"
#include "utils.h"
#include "thread_tests.h"

#define DEFAULT_THREADPOOL_SIZE 10
#define DEFAULT_SLEEP_DURATION_NSEC \
    500000000 // Waits half a second, converted to nsec

/**
 * @brief a mutex to protect shared memory in the test-provided
 * check_thread_working() function.
 * Not inteded for Trainee access or use.
 */
pthread_mutex_t test_mutex_g = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief a condition signal with to reduce busy waits in the test-provided
 * check_thread_working() function.
 * Not inteded for Trainee access or use.
 */
pthread_cond_t test_cond_g = PTHREAD_COND_INITIALIZER;

/**
 * @brief A function that counts the number of threads/workers that start the
 * function. Hangs until flag is set to false, then work is done. Stores the
 * pthread_id of each working thread.
 *
 * @param arg_p A void pointer to a thread_check_t struct with initialized
 * arguments.
 */
static void * check_thread_working(void * arg_p);

/**
 * @brief Basic function to increment an atomic int counter
 *
 * @param arg_p a pointer to an atomic int
 */
static void * increment_int(void * arg_p);

/**
 * @brief Function to check the pthread ids in the array
 * @attention Expects the size of thread_ids array to be one larger then
 * num_threads
 *
 * @param thread_ids an array of pthread_t ids
 * @param num_threads the number of threads believed to be working
 *
 * Function confirms that num_threads unique ids exist with the indices 0 to
 * num_threads-1 all unique, and thread_ids[num_threads] a duplicate id of
 * another thread
 */
static int check_unique_thread_id(pthread_t thread_ids[], uint32_t num_threads);

/**
 * @brief A helper struct to pass arguments to a "check_thread_working" job
 */
typedef struct thread_check_t
{
    atomic_int * counter;
    atomic_int * entry_counter;
    atomic_int * flag;
    pthread_t *  thread_id;
} thread_check_t;

static void * increment_int(void * arg_p)
{
    if (NULL == arg_p)
    {
        return NULL;
    }

    *(atomic_int *)arg_p += 1; // NOLINT
    return NULL;
}

static void * check_thread_working(void * arg_p)
{
    if (NULL == arg_p)
    {
        return NULL;
    }
    thread_check_t * args = (thread_check_t *)arg_p;
    pthread_mutex_lock(&test_mutex_g);
    (*(args->entry_counter))++; // NOLINT

    while (true == *(args->flag))
    {
        pthread_cond_wait(&test_cond_g, &test_mutex_g);
    }

    if (NULL != args->thread_id)
    {
        *(args->thread_id) = pthread_self();
    }

    // clang-diagnostic-unused-value
    pthread_mutex_unlock(&test_mutex_g);
    (*(args->counter))++; // NOLINT

    return NULL;
}

void test_threadpool_create(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_create);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_destroy);

    threadpool_t * pool_p = threadpool_create(DEFAULT_THREADPOOL_SIZE);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pool_p);
    CU_ASSERT_EQUAL(threadpool_destroy(&pool_p), SUCCESS);
    CU_ASSERT_PTR_NULL(pool_p);
}

void test_threadpool_add_job(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_create);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_add_job);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_destroy);

    threadpool_t * pool_p = threadpool_create(DEFAULT_THREADPOOL_SIZE);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pool_p);
    atomic_int * counter_p = calloc(1, sizeof(atomic_int));
    *counter_p             = 0;

    for (size_t i = 0; i < DEFAULT_THREADPOOL_SIZE; i++)
    {
        CU_ASSERT_EQUAL_FATAL(
            threadpool_add_job(pool_p, (job_f)increment_int, NULL, counter_p),
            SUCCESS);
    }

    CU_ASSERT_EQUAL(threadpool_destroy(&pool_p), SUCCESS);
    CU_ASSERT_PTR_NULL(pool_p);
    CU_ASSERT_EQUAL_FATAL(*counter_p, DEFAULT_THREADPOOL_SIZE);
    free(counter_p);
    counter_p = NULL;
}

void test_threadpool_add_job_null_pool(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_add_job);
    atomic_int counter = 0;
    int result = threadpool_add_job(NULL, (job_f)increment_int, NULL, &counter);
    CU_ASSERT_NOT_EQUAL(result, SUCCESS);
}

void test_threadpool_add_null_job(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_create);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_add_job);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_destroy);

    threadpool_t * pool_p = threadpool_create(DEFAULT_THREADPOOL_SIZE);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pool_p);

    // NULL job should fail
    int counter = 0;
    int result  = threadpool_add_job(pool_p, NULL, NULL, &counter);
    CU_ASSERT_NOT_EQUAL_FATAL(result, SUCCESS);

    // NULL del_f and NULL arg_p should succeed
    result = threadpool_add_job(pool_p, (job_f)increment_int, NULL, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(result, ERROR);

    // NULL arg_p should succeed
    result = threadpool_add_job(pool_p, (job_f)increment_int, free, NULL);
    CU_ASSERT_NOT_EQUAL_FATAL(result, ERROR);

    CU_ASSERT_EQUAL(threadpool_destroy(&pool_p), SUCCESS);
    CU_ASSERT_PTR_NULL(pool_p);
}

static int check_unique_thread_id(pthread_t thread_ids[], uint32_t num_threads)
{
    int all_unique = SUCCESS;

    int thread_id = 0;
    for (size_t idx = 0; idx < num_threads; idx++)
    {
        for (size_t j_idx = (idx + 1); j_idx < num_threads; j_idx++)
        {
            thread_id = pthread_equal(thread_ids[idx], thread_ids[j_idx]);
            if (0 != thread_id)
            {
                all_unique = ERROR;
                goto END;
            }
        }
    }

    for (uint32_t idx = 0; idx < num_threads; idx++)
    {
        if (pthread_equal(thread_ids[idx], thread_ids[num_threads]))
        {
            all_unique = (int)idx;
            if (0 == idx)
            {
                // if the duplicate thread_id was at index 0, set to 1 to show
                // duplicate
                all_unique = (int)idx + 1;
            }
            break;
        }
    }

END:
    return all_unique;
}

void test_threadpool_limit(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_create);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_add_job);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_destroy);

    threadpool_t * pool_p = threadpool_create(DEFAULT_THREADPOOL_SIZE);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pool_p);

    pthread_t * thread_ids =
        calloc(DEFAULT_THREADPOOL_SIZE + 1, sizeof(pthread_t));
    thread_check_t * thread_check =
        calloc(DEFAULT_THREADPOOL_SIZE + 1, sizeof(thread_check_t));

    if (NULL == thread_ids || NULL == thread_check)
    {
        threadpool_destroy(&pool_p);
        free(thread_ids);
        thread_ids = NULL;
        free(thread_check);
        thread_check = NULL;
        return;
    }

    atomic_int counter       = 0;
    atomic_int entry_counter = 0;
    atomic_int flag          = true;

    for (int i = 0; i < DEFAULT_THREADPOOL_SIZE + 1; i++)
    {
        thread_check[i].counter       = &counter;
        thread_check[i].entry_counter = &entry_counter;
        thread_check[i].flag          = &flag;
        thread_check[i].thread_id     = &thread_ids[i];
    }

    // add one more job than we have threads
    for (size_t i = 0; i < DEFAULT_THREADPOOL_SIZE + 1; i++)
    {
        CU_ASSERT_EQUAL_FATAL(threadpool_add_job(pool_p,
                                                 (job_f)check_thread_working,
                                                 NULL,
                                                 (void *)&thread_check[i]),
                              SUCCESS);
    }
    // give threads time to grab jobs - THIS IS BAD PRACTICE, TEST ONLY
    nanosleep((const struct timespec[]) { { 0, DEFAULT_SLEEP_DURATION_NSEC } },
              NULL);

    // set flag so threads do work
    pthread_mutex_lock(&test_mutex_g);
    *thread_check->flag = false;
    pthread_cond_broadcast(&test_cond_g);
    pthread_mutex_unlock(&test_mutex_g);

    CU_ASSERT_EQUAL(threadpool_destroy(&pool_p), SUCCESS);
    CU_ASSERT_PTR_NULL(pool_p);

    pthread_mutex_lock(&test_mutex_g);

    CU_ASSERT_EQUAL(*thread_check->counter, DEFAULT_THREADPOOL_SIZE + 1);

    // check that each thread did a job (minimum number of threads)
    int retval = check_unique_thread_id(thread_ids, DEFAULT_THREADPOOL_SIZE);
    CU_ASSERT_NOT_EQUAL(retval, ERROR);

    // check that one thread did two jobs (no more than maximum threads)
    CU_ASSERT_NOT_EQUAL(retval, SUCCESS);

    pthread_mutex_unlock(&test_mutex_g);

    free(thread_ids);
    thread_ids = NULL;
    free(thread_check);
    thread_check = NULL;
}

void test_threadpool_shutdown(void)
{
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_create);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_add_job);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_shutdown);
    CU_ASSERT_PTR_NOT_NULL_FATAL(threadpool_destroy);

    threadpool_t * pool_p = threadpool_create(DEFAULT_THREADPOOL_SIZE);
    CU_ASSERT_PTR_NOT_NULL_FATAL(pool_p);

    thread_check_t * thread_check =
        calloc(DEFAULT_THREADPOOL_SIZE + 1, sizeof(thread_check_t));

    if (NULL == thread_check)
    {
        free(thread_check);
        thread_check = NULL;
        threadpool_destroy(&pool_p);
        return;
    }

    atomic_int counter       = 0;
    atomic_int entry_counter = 0;
    atomic_int flag          = true;

    for (int i = 0; i < DEFAULT_THREADPOOL_SIZE + 1; i++)
    {
        thread_check[i].counter       = &counter;
        thread_check[i].entry_counter = &entry_counter;
        thread_check[i].flag          = &flag;
    }

    for (size_t i = 0; i < DEFAULT_THREADPOOL_SIZE + 1; i++)
    {
        CU_ASSERT_EQUAL_FATAL(
            threadpool_add_job(
                pool_p, (job_f)check_thread_working, NULL, thread_check),
            SUCCESS);
    }
    // sleep for threads to grab jobs -THIS IS BAD PRACTICE, TEST ONLY-
    nanosleep((const struct timespec[]) { { 0, DEFAULT_SLEEP_DURATION_NSEC } },
              NULL);

    // threadpool should be full from above loop b/c workers have made no
    // progress, but add job should succeed
    CU_ASSERT_EQUAL_FATAL(
        threadpool_add_job(
            pool_p, (job_f)check_thread_working, NULL, thread_check),
        SUCCESS);
    pthread_mutex_lock(&test_mutex_g);
    // assert that only default threadpool size number of threads have started
    CU_ASSERT_EQUAL(*thread_check->entry_counter, DEFAULT_THREADPOOL_SIZE);

    // start threads working
    *thread_check->flag = false;
    pthread_cond_broadcast(&test_cond_g);
    pthread_mutex_unlock(&test_mutex_g);

    // shutdown to threadpool
    CU_ASSERT_EQUAL_FATAL(threadpool_shutdown(pool_p), SUCCESS);

    // add job should fail after shutdown
    CU_ASSERT_EQUAL_FATAL(
        threadpool_add_job(
            pool_p, (job_f)check_thread_working, NULL, thread_check),
        ERROR);

    CU_ASSERT_EQUAL(threadpool_destroy(&pool_p), SUCCESS);
    CU_ASSERT_PTR_NULL(pool_p);

    // check that only pool_size + 2 jobs are started
    pthread_mutex_lock(&test_mutex_g);
    CU_ASSERT_EQUAL(*thread_check->entry_counter, DEFAULT_THREADPOOL_SIZE + 2);

    // check that only pool_size + 2 jobs are done
    CU_ASSERT_EQUAL(*thread_check->counter, DEFAULT_THREADPOOL_SIZE + 2);
    pthread_mutex_unlock(&test_mutex_g);

    free(thread_check);
    thread_check = NULL;
}
