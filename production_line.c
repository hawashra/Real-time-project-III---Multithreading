#include "includes/include.h"
#include "includes/functions.c"

pthread_t *employee_threads;


void* employee_routine(void* arg)
{
    while (1)
    {
        printf("Employee %ld is working\n", pthread_self());
        my_pause(1);
    }
    
    return NULL;
}

int main(int argc, char const *argv[])
{

    int employee_count = atoi (argv[1]);

    employee_threads = (pthread_t*)malloc(employee_count * sizeof(pthread_t));

    for (int i = 0; i < employee_count; i++)
    {
        pthread_create(&employee_threads[i], NULL, employee_routine, NULL);
    }

    for (int i = 0; i < employee_count; i++)
    {
        pthread_join(employee_threads[i], NULL);
    }

    return 0;
}
