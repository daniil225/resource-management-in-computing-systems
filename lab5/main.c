#include <stdio.h>
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork(), sleep(), usleep()
#include <stdlib.h>    // fprintf(), fscanf()
#include <inttypes.h> 
#include <sys/resource.h>

#define MAX_COUNT_PID 5

pid_t Pids[MAX_COUNT_PID]; // Массив идентификаторов процесса 


int main()
{

    if(setpriority(PRIO_PROCESS, getpid(), -1) < 0)
    {
        perror("setpriority");
    }


    int priority = getpriority(PRIO_PROCESS, getpid());

    printf("Priority = %d\n", priority);

    return 0;
}