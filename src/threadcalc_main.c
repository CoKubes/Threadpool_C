#include "threadpool.h"
#include <unistd.h>
#include <bits/getopt_core.h>
#include "filecalc.h"

#define DEFAULT_THREAD_COUNT 4

void * filecalc_job(void * arg)
{
    char ** params           = (char **)arg;
    char *  binary_file      = params[0];
    char *  target_directory = params[1];

    // Create a single element array to simulate the binary_files list
    char ** binary_files = &binary_file;
    int     file_count   = 1;

    handle_files(&binary_files, &file_count, target_directory);

    return NULL;
}

int main(int argc, char * argv[])
{
    int            error_status = SUCCESS;
    int            thread_count = DEFAULT_THREAD_COUNT;
    int            opt;
    threadpool_t * pool         = NULL;
    char **        binary_files = NULL;
    int            file_count   = 0;
    char *         dir_source   = NULL;
    char *         dir_target   = NULL;

    // Initialize signal handling
    if (signal_action_setup() != 0)
    {
        fprintf(stderr, "Failed to set up signal handling\n");
        error_status = ERROR;
    }

    if (SUCCESS == error_status)
    {
        while ((opt = getopt(argc, argv, "n:")) != ERROR)
        {
            switch (opt)
            {
                case 'n':
                    thread_count = atoi(optarg);
                    if (thread_count <= 0)
                    {
                        fprintf(stderr, "Invalid number of threads: %s\n", optarg);
                        error_status = ERROR;
                    }
                    break;
                default:
                    fprintf(stderr, "Usage: %s [-n thread_count] source_dir target_dir\n", argv[0]);
                    error_status = ERROR;
                    break;
            }

            if (SUCCESS != error_status)
                break;
        }
    }

    if (SUCCESS == error_status)
    {
        if (argc - optind != 2)
        {
            fprintf(stderr, "Usage: %s [-n thread_count] source_dir target_dir\n", argv[0]);
            error_status = ERROR;
        }
    }

    if (SUCCESS == error_status)
    {
        dir_source = argv[optind];
        dir_target = argv[optind + 1];

        error_status = get_files(dir_source, &binary_files, &file_count);
        if (SUCCESS != error_status)
        {
            get_error(error_status);
        }
    }

    if (SUCCESS == error_status)
    {
        error_status = check_permissions(&binary_files, &file_count);
        if (SUCCESS != error_status)
        {
            get_error(error_status);
        }
    }

    if (SUCCESS == error_status)
    {
        pool = threadpool_create(thread_count);
        if (NULL == pool)
        {
            fprintf(stderr, "Error creating threadpool\n");
            error_status = ERROR;
        }
    }

    if (SUCCESS == error_status)
    {
        for (int i = 0; i < file_count; i++)
        {
            char ** job_params = malloc(2 * sizeof(char *));
            if (NULL == job_params)
            {
                fprintf(stderr, "Memory allocation error\n");
                error_status = ERROR;
                break;
            }
            job_params[0] = binary_files[i];
            job_params[1] = dir_target;

            if (SUCCESS != threadpool_add_job(pool, filecalc_job, free, job_params))
            {
                fprintf(stderr, "Error adding job to threadpool\n");
                free(job_params);
            }
        }
    }

    if (pool)
    {
        threadpool_destroy(&pool);
    }
    if (binary_files)
    {
        free_list(&binary_files, &file_count);
    }

    return error_status;
}
