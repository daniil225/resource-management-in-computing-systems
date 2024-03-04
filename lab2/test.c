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

double _exp(double x);


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
        //printf("Prepared data: pid = %d, f = %lf\n", fndata.pid, fndata.f);
        fprintf(fp, "%d %lf", fndata.pid, fndata.f);
        sleep(1);
        fclose(fp);
        exit(0);
    }
    else
    {

        struct FuncData fndata = {0, 0};
        int i = 0;
        printf("Try read...\n");
        while (fndata.pid != pid)
        {
            FILE *fp = fopen("InterProcess.txt", "r");

            if (fp == NULL)
            {
                fprintf(stderr, "File cannot open\n");
                exit(-1);
            }
            fscanf(fp, "%d %lf", &fndata.pid, &fndata.f);
            i++;
            fclose(fp);
            usleep(50000); // задержка на 0.05 сек
        }
        printf("Total iteration: %d\n", i);
        printf("Transport data: pid = %d pidvariable = %d f = %lf\n", fndata.pid, pid, fndata.f);
        
    }

    //
    // system("rm InterProcess");

    return 0;
}