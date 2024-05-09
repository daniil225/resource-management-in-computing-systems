#include <stdio.h>
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork(), sleep(), usleep()
#include <stdlib.h>    // fprintf(), fscanf()
#include <inttypes.h> 
#include <sys/resource.h>
#include <sys/wait.h>   // waitpid
#include <sys/signal.h> // signal, kill

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define PID_POOL 100
#define MAX_CURR_PID 3
#define MSG_BUFFER_SIZE 256
#define DELAY_TIME 1000 // Время сна перед очередной проверкой для процесса 
#define SHM_KEY 1234

pid_t pidPool[PID_POOL]; // Массив идентификаторов процесса 
pid_t currPid[MAX_CURR_PID]; // Массив максимального количества одновременно запущенных программ

/* Структура управления действиями процесса */
typedef struct  _pidCommand
{
    int32_t type_action; // Тип действия 0 - ничего не делать, 1 - загрузить данные из очереди, 2 - загрузить данные в очередб 3 - завершить процесс
    pid_t pid; // Процесс которому предназначено действие 
    int32_t CanSend; //  Можно ли отправлять запрос 
}PidCommand;

PidCommand *pid_command;


void alarm_handler(int sig){ } // Функция пустышка для пробуждения процесса 

int main()
{

    signal(SIGALRM, alarm_handler);
    /* Создать сегмент разделаемой памяти */
    int shm_id = shmget(SHM_KEY, sizeof(PidCommand), IPC_CREAT | 0666);
    if(shm_id == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Присоединяем сегмент разделаемой памяти к своему адресному пространству 
    void *shm_addr = shmat(shm_id, NULL, 0);
    if(shm_addr == (void*)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    pid_command = (PidCommand*)shm_addr;

    pid_t pid;

    if((pid = fork()) == 0)
    {
        // Цикл процесса 
        while(1)
        {
            if(pid_command->pid == getpid())
            {
                if(pid_command->type_action == 1)
                {
                    printf("Action 1\n");
                    pid_command->CanSend = 1;
                }
                else if(pid_command->type_action == 2)
                {
                    printf("Action 2\n");
                    pid_command->CanSend = 1;
                }
                else if(pid_command->type_action == 3)
                {
                    printf("Action 3\n");
                    pid_command->CanSend = 1;
                    break;
                }
            }
            sleep(DELAY_TIME); // Сон, не поедаем процессорное время. Раз в 1000сек смотрим не изменилось ли состояние и не пора ли нам чего то сделать 
        }

        printf("Child pid = %d\n", getpid());
        execl("prog1", "", NULL);
        perror("execl");
    }
    else  if(pid < 0)
    {   
        
        perror("Fork error\n");
        exit(-1);
    }

    pid_command->pid = pid;
    pid_command->type_action = 1;
    pid_command->CanSend = 0;

    while(kill(pid, SIGALRM));
    while(pid_command->CanSend == 0); // Пока ноль не идем далее    
    
    pid_command->type_action = 2;
    pid_command->CanSend = 0;
    

    while(kill(pid, SIGALRM));
    while(pid_command->CanSend == 0); // Пока ноль не идем далее    
    
    
    pid_command->type_action = 3;
    while(kill(pid, SIGALRM));
    while(pid_command->CanSend == 0); // Пока ноль не идем далее    
    

    int status;
    waitpid(pid, &status, 0);

    // Отсоединить сегмент разделяемой памяти от своего адресного пространства
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    // Удалить сегмент разделяемой памяти
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}