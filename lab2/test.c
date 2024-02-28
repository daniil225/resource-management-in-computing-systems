/* Стартовая попытка испольщовать процессы для выполнения какой либо задачи */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

struct FuncData
{
    pid_t pid;
    double f;
};

int main()
{

    pid_t pid = fork();

    if (pid == 0)
    {
        FILE *fp = fopen("InterProcess.txt", "w");

        if (fp == NULL)
        {
            fprintf(stderr, "File cannot open\n");
            exit(-1);
        }
        struct FuncData fndata = {getpid(), 34444.5};
        printf("Prepared data: pid = %d, f = %lf\n", fndata.pid, fndata.f);
        fprintf(fp, "%d",fndata.pid);
        fclose(fp);
        exit(0);
    }
    else
    {
        FILE *fp = fopen("InterProcess.txt", "r");

        if (fp == NULL)
        {
            fprintf(stderr, "File cannot open\n");
            exit(-1);
        }
        struct FuncData fndata = {0, 0};
        int i = 0;
        //sleep(3);
        wait(NULL); // Синхронизация с процессов 
        while (fndata.pid != pid)
        {
            fscanf(fp, "%d", &fndata.pid);
            i++;
            
            printf("Read...\n");
            sleep(1);
        }
        printf("Total iteration: %d\n", i);
        printf("Transport data: pid = %d pidvariable = %d f = %lf\n", fndata.pid, pid, fndata.f);
        fclose(fp);
    }


    //
    // system("rm InterProcess");

    return 0;
}