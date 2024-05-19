#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    sleep(3);
    printf("pid = %d Work lab4\n", getpid());
    

    exit(0);
}