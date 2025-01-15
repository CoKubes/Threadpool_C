#ifndef THREAD_TESTS_H
#define THREAD_TESTS_H

/**
 * @brief Test create new threadpool.
 *
 */
void test_threadpool_create(void);

/**
 * @brief Test queueing a job in the threadpool.
 *
 */
void test_threadpool_add_job(void);

/**
 * @brief Test queue job handles null pool.
 *
 */
void test_threadpool_add_job_null_pool(void);

/**
 * @brief Test queue job handles null job.
 *
 */
void test_threadpool_add_null_job(void);

/**
 * @brief Test limits of threadpool.
 *
 */
void test_threadpool_limit(void);

/**
 * @brief Test shutdown threadpool.
 *
 */
void test_threadpool_shutdown(void);

#endif
