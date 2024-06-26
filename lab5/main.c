#include <stdio.h>
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork(), sleep(), usleep()
#include <stdlib.h>    // fprintf(), fscanf()
#include <inttypes.h>
#include <sys/resource.h>
#include <sys/wait.h>   // waitpid
#include <sys/signal.h> // signal, kill
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>

#define PID_POOL 100
#define MAX_CURR_PID 3
#define MSG_BUFFER_SIZE 256
#define DELAY_TIME 1000 // Время сна перед очередной проверкой для процесса
#define SHM_KEY 101
#define MSG_KEY 16
#define SEM_KEY 1235
#define BREAK_PROC 2
#define LOAD_PROC_CYCLE_WORK_TO_QUEUE 1

pid_t pidPool[PID_POOL];     // Массив идентификаторов процесса
pid_t currPid[MAX_CURR_PID]; // Массив максимального количества одновременно запущенных программ

/* Структура управления действиями процесса */
typedef struct _pidCommand
{
    int32_t type_action; // Тип действия 0 - ничего не делать, 1 - загрузить данные из очереди, 2 - завершить процесс
    pid_t pid;           // Процесс которому предназначено действие
    int32_t CanSend;     //  Можно ли отправлять запрос
} PidCommand;

typedef struct _msg_buf
{
    long mtype;
    char mtext[MSG_BUFFER_SIZE];
} MsgBuf;

PidCommand *pid_command;

void alarm_handler(int sig) {} // Функция пустышка для пробуждения процесса

void send_proc_command(pid_t pid, int32_t action_type)
{
    // printf("set pid = %d, type = %d\n", pid, action_type);
    while (pid_command->pid != pid)
        pid_command->pid = pid;
    while (pid_command->type_action != action_type)
        pid_command->type_action = action_type;
    while (pid_command->CanSend != 0)
        pid_command->CanSend = 0;
    while (kill(pid, SIGALRM))
        ;
    while (pid_command->CanSend == 0)
        ;
}


int main()
{
    pid_t parentpid = getpid();
    /* Создаем набор из 1 семафора */
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if(sem_id == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Инициализация семафора начальным значением 1
    union semun
    {
        int val;
        struct semid_ds  *bug;
        unsigned short *array;
        
    }sem_union;
    sem_union.val = 1;
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        perror("semctl");
        exit(EXIT_FAILURE);
    }


    // Переопределить поведение сигнала
    signal(SIGALRM, alarm_handler);

    /* Создать сегмент разделаемой памяти */
    int shm_id = shmget(SHM_KEY, sizeof(PidCommand), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Присоединяем сегмент разделаемой памяти к своему адресному пространству
    void *shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    /* Создадим очередь сообщений */
    int32_t msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    pid_command = (PidCommand *)shm_addr;

    if ((pidPool[0] = fork()) == 0)
    {
        // Цикл процесса
        const int SIZE = 6;
        char *CycleWork[6] = {"lab1", "lab2", "lab3", "lab4", "lab5", "lab1"};

        while (1)
        {
            // printf("cycle....");
            // printf("pid = %d action = %d\n", pid_command->pid, pid_command->type_action);

            if (pid_command->pid == getpid())
            {
                if (pid_command->type_action == 1)
                {
                    for (int i = 0; i < SIZE; i++)
                    {
                        MsgBuf msg;
                        msg.mtype = 1;
                        strcpy(msg.mtext, CycleWork[i]);
                        if (msgsnd(msgid, &msg, sizeof(msg.mtext), 1) == -1)
                        {
                            perror("msgsnd");
                            exit(EXIT_FAILURE);
                        }

                        usleep(500);
                    }
                    printf("Proc pid = %d Load was end\n", getpid());
                    /* Подъем флага синхронизации */
                    while (pid_command->CanSend == 0)
                        pid_command->CanSend = 1;
                }
                else if (pid_command->type_action == 2)
                {
                    printf("Proc pid = %d was end\n", getpid());
                    while (pid_command->CanSend == 0)
                        pid_command->CanSend = 1;
                    break;
                }
            }
            sleep(DELAY_TIME); // Сон, не поедаем процессорное время. Раз в 1000сек смотрим не изменилось ли состояние и не пора ли нам чего то сделать
        }
    }
    else if (pidPool[0] < 0)
    {

        perror("Fork error\n");
        exit(-1);
    }

    if ((pidPool[1] = fork()) == 0)
    {
        // Цикл процесса
        const int SIZE = 6;
        char *CycleWork[6] = {"lab1", "lab2", "lab3", "lab4", "lab5", "lab1"};

        // Цикл процесса
        while (1)
        {
            // printf("cycle....");
            // printf("pid = %d action = %d\n", pid_command->pid, pid_command->type_action);

            if (pid_command->pid == getpid())
            {
                if (pid_command->type_action == 1)
                {
                    for (int i = 0; i < SIZE; i++)
                    {
                        MsgBuf msg;
                        msg.mtype = 1;
                        strcpy(msg.mtext, CycleWork[i]);
                        if (msgsnd(msgid, &msg, sizeof(msg.mtext), 1) == -1)
                        {
                            perror("msgsnd");
                            exit(EXIT_FAILURE);
                        }
                        usleep(500);
                    }
                    printf("Proc pid = %d Load was end\n", getpid());
                    /* Подъем флага синхронизации */
                    while (pid_command->CanSend == 0)
                        pid_command->CanSend = 1;
                }
                else if (pid_command->type_action == 2)
                {
                    printf("proc pid = %d was end\n", getpid());
                    while (pid_command->CanSend == 0)
                        pid_command->CanSend = 1;
                    break;
                }
            }
            sleep(DELAY_TIME); // Сон, не поедаем процессорное время. Раз в 1000сек смотрим не изменилось ли состояние и не пора ли нам чего то сделать
        }
    }
    else if (pidPool[1] < 0)
    {

        perror("Fork error\n");
        exit(-1);
    }

    /* Основной цикл работ */
    /* Загружаем в очередь весь список для выполнения */

    if (pidPool[0] != 0 && pidPool[1] != 0)
    {

        send_proc_command(pidPool[0], LOAD_PROC_CYCLE_WORK_TO_QUEUE);
        send_proc_command(pidPool[1], LOAD_PROC_CYCLE_WORK_TO_QUEUE);
        /* Завершаем процессы */
        send_proc_command(pidPool[0], BREAK_PROC);
        send_proc_command(pidPool[1], BREAK_PROC);

        /* Выполняем все программы из очереди */
        /*всего их в очереди 10 штук поэтому загружем ровно 10 штук*/

        int prog_cnt = 12;

        while (prog_cnt > 0)
        {
            /* Создадим 3 процесса для выполнения программы */
            pid_t pid1;
            pid_t pid2;
            pid_t pid3;

            if((pid1 = fork()) == 0)
            {
                /*  Захват семафора */
                struct sembuf sem_op;
                sem_op.sem_num = 0;
                sem_op.sem_op = -1;
                sem_op.sem_flg = 0;
                if(semop(sem_id, &sem_op, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }
                /* Критическая секция */

                /* Извлекаем данные из очереди */
                MsgBuf msg;
                if (msgrcv(msgid, &msg, MSG_BUFFER_SIZE, 1, 0) == -1)
                {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                
                /* Освободить семафор */
                sem_op.sem_op = 1;
                if(semop(sem_id, &sem_op, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }

                printf("pid1 = %d: %s\n",getpid(),msg.mtext); 
                if(execl(msg.mtext, "", NULL) == -1)
                {
                    perror("execl");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
            }
            else if((pid2 = fork()) == 0 && pid1 > 0)
            {
                /*  Захват семафора */
                struct sembuf sem_op;
                sem_op.sem_num = 0;
                sem_op.sem_op = -1;
                sem_op.sem_flg = 0;
                if(semop(sem_id, &sem_op, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }

                /* Критическая секция */

                /* Извлекаем данные из очереди */
                MsgBuf msg;
                if (msgrcv(msgid, &msg, MSG_BUFFER_SIZE, 1, 0) == -1)
                {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                /* Освободить семафор */
                sem_op.sem_op = 1;
                if(semop(sem_id, &sem_op, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }

               printf("pid2 = %d: %s\n",getpid(),msg.mtext); 
                if(execl(msg.mtext, "", NULL) == -1)
                {
                    perror("execl");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
            }
            else if((pid3 = fork()) == 0 && pid1 > 0 && pid2 > 0)
            {
                /*  Захват семафора */
                struct sembuf sem_op;
                sem_op.sem_num = 0;
                sem_op.sem_op = -1;
                sem_op.sem_flg = 0;
                if(semop(sem_id, &sem_op, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }

                /* Критическая секция */

                /* Извлекаем данные из очереди */
                MsgBuf msg;
                if (msgrcv(msgid, &msg, MSG_BUFFER_SIZE, 1, 0) == -1)
                {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                
                /* Освободить семафор */
                sem_op.sem_op = 1;
                if(semop(sem_id, &sem_op, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }


               printf("pid3 = %d: %s\n",getpid(),msg.mtext); 
                if(execl(msg.mtext, "", NULL) == -1)
                {
                    perror("execl");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
            }
            else if(pid1 > 0 && pid2 > 0 && pid3 > 0)
            {
                int status;
                //printf("Parentpid  ");
                //printf("pid1 = %d  pid2 = %d\n", pid1, pid2); 

                if(waitpid(pid1, &status, 0) == pid1)
                { 
                    //printf("status pid1 = %d\n", status);
                    prog_cnt--;
                }
           
                if(waitpid(pid2, &status, 0) == pid2)
                {
                    //printf("status pid2 = %d\n", status);
                    prog_cnt--;
                } 

                if(waitpid(pid3, &status, 0) == pid3)
                {
                    prog_cnt--;
                }
                    
               //printf("prog_cnt: %d\n", prog_cnt);
            
            }
            else 
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            //prog_cnt--;
        }

        waitpid(pidPool[0], NULL, 0);
        waitpid(pidPool[1], NULL, 0);

        // Отсоединить сегмент разделяемой памяти от своего адресного пространства
        if (shmdt(shm_addr) == -1)
        {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }

        // Удалить сегмент разделяемой памяти
        if (shmctl(shm_id, IPC_RMID, NULL) == -1)
        {
            perror("shmctl");
            exit(EXIT_FAILURE);
        }

        /* Закрыть очередь сообщений */
        if (msgctl(msgid, IPC_RMID, NULL) == -1)
        {
            perror("msgctl");
            exit(EXIT_FAILURE);
        }

        /* Удалить набор семафоров */
        if(semctl(sem_id, 0, IPC_RMID) == -1)
        {
            perror("semctl");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}