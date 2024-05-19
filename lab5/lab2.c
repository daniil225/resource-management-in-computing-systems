#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    sleep(1);
    printf("pid = %d Work lab2\n", getpid());
    
    exit(0);
}