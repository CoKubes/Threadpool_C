#include "signals.h"
#include "utils.h"

// Define the global signal flag
volatile sig_atomic_t signal_flag_g = 0;

void signal_handler(int signal)
{
    if (SIGUSR1 == signal)
    {
        signal_flag_g = 1;
    }
    else if (SIGINT == signal)
    {
        signal_flag_g = 2;
    }
}

int signal_action_setup(void)
{
    int retval = SUCCESS_;
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags   = 0; // or SA_RESTART

    sigemptyset(&sa.sa_mask);

    if (ERROR == sigaction(SIGUSR1, &sa, NULL))
    {
        perror("sigaction");
        retval = ERROR;;
    }

    if (ERROR == sigaction(SIGINT, &sa, NULL))
    {
        perror("sigaction");
        retval = ERROR;
    }

    return retval;
}
