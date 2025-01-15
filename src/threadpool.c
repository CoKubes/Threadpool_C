#include "threadpool.h" //../include/threadpool.h

typedef struct
{
    job_f  job_function;
    void * job_arg;
    free_f job_free_function;
} job_t;

void * worker_function(void * arg)
{
    threadpool_t * pool = (threadpool_t *)arg; // Cast the argument to a threadpool_t pointer

    while (1)
    {
        pthread_mutex_lock(&pool->queue_lock); // Lock the queue to ensure thread-safe access

        // Wait for jobs if the queue is empty and stop flag is not set
        while (queue_emptycheck(pool->job_queue) && !pool->stop)
        {
            // Wait for the condition variable to be signaled, releasing the mutex during the wait
            pthread_cond_wait(&pool->queue_cond, &pool->queue_lock);
        }

        // If the stop flag is set and the queue is empty, exit the loop
        if (pool->stop && queue_emptycheck(pool->job_queue))
        {
            pthread_mutex_unlock(&pool->queue_lock); // Unlock the queue before exiting
            break;
        }

        // Dequeue a job from the job queue
        queue_node_t * node = queue_dequeue(pool->job_queue);
        pthread_mutex_unlock(&pool->queue_lock); // Unlock the queue after dequeueing

        if (NULL != node)
        {
            job_t * job = (job_t *)node->data; // Cast the node's data to job_t
            if (NULL != job)
            {
                job->job_function(job->job_arg); // Execute the job function
                free(job);                       // Free the job structure
            }
            free(node); // Free the node structure
        }
        // Check for signal flag
        if (signal_flag_g == 1)
        {
            printf("Received SIGUSR1, performing custom action...\n");
            signal_flag_g = 0; // Reset flag after handling
        }
        else if (signal_flag_g == 2)
        {
            printf("Received SIGINT, shutting down...\n");
            threadpool_shutdown(pool); // Gracefully shutdown
            break;
        }
    }

    return NULL;
}

/**
 * @brief Create a new threadpool and instantiate as required.
 *
 * @param thread_count The number of threads to create in the threadpool
 *
 * @return SUCCESS: A threadpool instance of type threadpool_t.
 *         FAILURE: NULL
 */
threadpool_t * threadpool_create(size_t thread_count)
{
    threadpool_t * pool       = NULL;    // Initialize the pool pointer to NULL
    int            error_code = SUCCESS; // Initialize error code to SUCCESS

    do
    {
        // Allocate memory for the threadpool structure
        pool = (threadpool_t *)malloc(sizeof(threadpool_t));
        if (NULL == pool)
        {
            error_code = ERROR;
            break;
        }

        // Allocate memory for the threads
        pool->threads = (pthread_t *)malloc(thread_count * sizeof(pthread_t));
        if (NULL == pool->threads)
        {
            error_code = ERROR;
            break;
        }

        pool->thread_count = thread_count; // Set the thread count
        pool->stop         = 0;            // Initialize the stop flag to 0

        // Initialize the job queue with twice the number of threads
        pool->job_queue = queue_init(100, free); // Arbitrary size
        if (NULL == pool->job_queue)
        {
            error_code = ERROR;
            break;
        }

        // Initialize the mutex lock
        if (SUCCESS != pthread_mutex_init(&pool->queue_lock, NULL))
        {
            error_code = ERROR;
            break;
        }

        // Initialize the condition variable
        if (SUCCESS != pthread_cond_init(&pool->queue_cond, NULL))
        {
            error_code = ERROR;
            break;
        }

        // Create the worker threads
        for (size_t i = 0; i < thread_count; i++)
        {
            if (SUCCESS != pthread_create(&pool->threads[i], NULL, worker_function, (void *)pool))
            {
                error_code = ERROR;
                break;
            }
        }

        if (ERROR == error_code)
        {
            break;
        }

    } while (0);

    // Cleanup in case of error; I dont love this but im trying really hard to not use multiple returns
    if (ERROR == error_code)
    {
        if (NULL != pool)
        {
            if (pool->threads)
            {
                free(pool->threads);
            }
            if (pool->job_queue)
            {
                queue_destroy(&pool->job_queue);
            }
            pthread_mutex_destroy(&pool->queue_lock);
            pthread_cond_destroy(&pool->queue_cond);
            free(pool);
            pool = NULL;
        }
    }

    return pool;
}

int threadpool_shutdown(threadpool_t * pool_p)
{
    int retval = ERROR;

    if (NULL != pool_p)
    {
        pthread_mutex_lock(&pool_p->queue_lock);     // Lock the queue to ensure thread-safe access
        pool_p->stop = 1;                            // Set the stop flag to indicate shutdown
        pthread_cond_broadcast(&pool_p->queue_cond); // Wake up all worker threads
        pthread_mutex_unlock(&pool_p->queue_lock);   // Unlock the queue

        // Join all worker threads to ensure they have finished executing
        for (size_t i = 0; i < pool_p->thread_count; i++)
        {
            pthread_join(pool_p->threads[i], NULL);
        }
        retval = SUCCESS;
    }

    return retval;
}

int threadpool_destroy(threadpool_t ** pool_pp)
{
    int retval = ERROR;

    if (NULL != pool_pp && NULL != *pool_pp)
    {
        threadpool_t * pool = *pool_pp;
        retval              = threadpool_shutdown(pool); // Shutdown the threadpool

        if (SUCCESS == retval)
        {
            pthread_mutex_destroy(&pool->queue_lock); // Destroy the mutex lock
            pthread_cond_destroy(&pool->queue_cond);  // Destroy the condition variable
            queue_destroy(&pool->job_queue);          // Destroy the job queue
            free(pool->threads);                      // Free the threads array
            free(pool);                               // Free the threadpool structure
            *pool_pp = NULL;                          // Set the double pointer to NULL
        }
    }

    return retval;
}

int threadpool_add_job(threadpool_t * pool_p, job_f job, free_f del_f, void * arg_p)
{
    int result = ERROR;

    if (pool_p && job)
    {
        // Allocate memory for the new job
        job_t * new_job = (job_t *)malloc(sizeof(job_t));

        if (NULL != new_job)
        {
            new_job->job_function      = job;   // Set the job function
            new_job->job_arg           = arg_p; // Set the job argument
            new_job->job_free_function = del_f; // Set the job's free function

            pthread_mutex_lock(&pool_p->queue_lock); // Lock the queue to ensure thread-safe access
            if (SUCCESS == queue_enqueue(pool_p->job_queue, new_job))
            {
                pthread_cond_signal(&pool_p->queue_cond); // Signal a waiting thread that a job is available
                result = SUCCESS;
            }
            else
            {
                free(new_job); // Free the new job if enqueueing fails
            }
            pthread_mutex_unlock(&pool_p->queue_lock); // Unlock the queue
        }
    }

    return result;
}