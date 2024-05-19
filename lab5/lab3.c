#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    sleep(2);
    printf("pid = %d Work lab3\n", getpid());
    

    exit(0);
}