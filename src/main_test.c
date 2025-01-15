#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/threadpool.h"

// Define a job function that adds two numbers
void* add_numbers(void *arg) {
    int *args = (int *)arg;
    int num1 = args[0];
    int num2 = args[1];
    sleep(1);
    printf("Thread 0x%lx is Adding numbers: %d + %d = %d\n", (unsigned long)pthread_self(), num1, num2, num1 + num2);
    free(arg); // Free the allocated memory for arguments
}

int main() {
    // Initialize threadpool with 4 threads
    threadpool_t *pool = threadpool_create(4);
    if (pool == NULL) {
        fprintf(stderr, "Error: Failed to create threadpool.\n");
        return EXIT_FAILURE;
    }

    // Add 100 jobs to the threadpool
    for (int i = 0; i < 10; i++) {
        // Allocate memory for job arguments
        int *args = (int *)malloc(2 * sizeof(int));
        if (args == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            threadpool_destroy(&pool);
            return EXIT_FAILURE;
        }

        // Set arguments for the job
        args[0] = i;        // Example number 1
        args[1] = i + 1;    // Example number 2
        //printf("arg1 %d, arg2 %d\n", args[0],args[1]);
        // Add job to the threadpool
        if (threadpool_add_job(pool, add_numbers, free, (void *)args) != SUCCESS) {
            fprintf(stderr, "Error: Failed to add job to threadpool.\n");
            free(args); // Clean up memory if job addition fails
            threadpool_destroy(&pool);
            return EXIT_FAILURE;
        }
    }

    // Sleep to allow jobs to be processed
    sleep(5);

    // Shutdown and destroy threadpool
    if (threadpool_destroy(&pool) != SUCCESS) {
        fprintf(stderr, "Error: Failed to destroy threadpool.\n");
        return EXIT_FAILURE;
    }

    printf("Yuh\n");
    return EXIT_SUCCESS;
}
