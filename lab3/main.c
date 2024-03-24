#include <stdio.h>     // io-functions
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork(), sleep(), usleep()
#include <stdlib.h>    // fprintf(), fscanf()
#include <string.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <sys/signal.h>

#define SIGBREAK SIGUSR2 // Завершает P0

int state = 0;

struct _Data
{
    pid_t pid;
    char data[40];
};

void handler(int sig)
{
    if (sig == SIGCONT)
    {
        // printf("signal set 1\n");
        state = 1; // Процесс готов начать обработку данных
    }
    else if (sig == SIGBREAK)
    {
        state = 3; //  Команда завершения цикла
    }
}

typedef struct _Data Data;

int main()
{

    /* Регистрация обработчиков сигналов */
    signal(SIGCONT, handler);
    signal(SIGBREAK, handler);

    pid_t P1; // ID 1 ого процесса
    pid_t P2; // ID 2 ого процесса порожденного в 1-ом

    int32_t K1[2]; // Програмный канал номер 1; K1[0] - дескриптор для чтения, K1[1] - дескриптор для записи
    int32_t K2[2]; // Програмный канал номер 2; Аналогично K1

    /* Инициализация програмныйх каналов */
    printf("P0: Try to create K1\n");
    if (pipe(K1))
    {
        fprintf(stderr, 'P0: Pipe failed.\n');
        return EXIT_FAILURE;
    }
    printf("P0: K1 create success\n");

    printf("P0: Try to create K2\n");
    if (pipe(K2))
    {
        fprintf(stderr, 'P0: Pipe failed.\n');
        return EXIT_FAILURE;
    }
    printf("P0: K2 create success\n\n");

    /*************************************/

    // Создание процесса P1
    printf("P0: Try to create P1\n");
    if ((P1 = fork()) == 0)
    {
        usleep(1000); // тактирование

        printf("P1: P1 create sucess. pid(P1) = %d\n", P1);

        printf("P1: Try to create P2\n");
        if ((P2 = fork()) == 0)
        {
            printf("P2: P2 create sucess. pid(P2) = %d\n", P2);
            // Подготовка данных
            Data dataP2 = {getpid(), "Data: P2"};
            write(K2[1], &dataP2.pid, 4);
            write(K2[1], dataP2.data, 40);
            printf("P2(2): Data writen and sended to K2\n");
            exit(EXIT_SUCCESS);
        }
        else if (P2 < (pid_t)0)
        {
            /* The fork failed. */
            fprintf(stderr, 'P1: Fork failed.\n');
            exit(EXIT_FAILURE);
        }

        // Подготовка данных
        Data dataP1 = {getpid(), "Data: P1"}; // Подготовленные данные для канала K1 от P1

        // Пишем данные в канал K1 P1(1)
        write(K1[1], &dataP1.pid, 4);
        write(K1[1], dataP1.data, 40);
        printf("P1(1): Data wreaten and send to K1\n");
        kill(getppid(), SIGCONT); // state = 1; Сигнал отправки данных
        usleep(100);              // Тактирование
        /************************************************************/

        /* Ждем корректного завершения P2 */
        int statusP2;
        waitpid(P2, &statusP2, NULL);
        if (statusP2 == 0)
        {
            printf("P1: Process P2 was end correct\n");
        }
        else
        {
            fprintf(stderr, "P1: P2 error status = %d\n", statusP2);
            return EXIT_FAILURE;
        }
        /* Уверенны, что данные в K2 уже есть читаем их и сохраняем */
        Data dataP2;
        read(K2[0], &dataP2.pid, 4);
        read(K2[0], dataP2.data, 40);
        // Пишем эту порцию данных в K1 и посылаем сигнал P0 на прием
        write(K1[1], &dataP2.pid, 4);
        write(K1[1], dataP2.data, 40);
        printf("P1(3): Data from P2 got and written to K1 and sended\n");
        kill(getppid(), SIGCONT);
        usleep(100); // Тактирование

        /* Формируем 4 набор данных для P0 */
        strcat(dataP1.data, " ");
        strcat(dataP1.data, dataP2.data);
        // Пишем данные в канал K1 P1(1)
        write(K1[1], &dataP1.pid, 4);
        write(K1[1], dataP1.data, 40);
        printf("P1(4): Data P1 modifie and send to K1\n");
        kill(getppid(), SIGCONT); // state = 1; Сигнал отправки данных
        usleep(100);              // Тактирование
        /* Выход из цикла в P0 - можно было это на waitpid повесить, но так хотя бы сигналы поиспользовали */
        usleep(100);
        kill(getppid(), SIGBREAK);

        exit(EXIT_SUCCESS);
    }
    // Не удалось создать дочерний процесс
    else if (P1 < (pid_t)0)
    {
        /* The fork failed. */
        fprintf(stderr, 'P0: Fork failed.\n');
        exit(EXIT_FAILURE);
    }
    // Родитель
    else
    {
        int32_t status;
        /* Ждем сигналов от потомка и читаем данные из канала K1 */
        /* Обработчик для чтения */

        while (1)
        {
            if (state == 1)
            {
                Data data;
                read(K1[0], &data.pid, 4);
                read(K1[0], data.data, 40);
                printf("P0: pid = %d, data = %s\n", data.pid, data.data);
                state = 0;
            }
            else if (state == 3)
            {
                printf("P0: break;\n");
                break;
            }
        }

        waitpid(P1, &status, NULL); // Ждем P1, когда он завершится читаем данные из канала K1
        if (status == 0)
        {
            printf("P0: Process P1 was end correct\n");
        }
        else
        {
            fprintf(stderr, "P0: P1 error status = %d\n", status);
            return EXIT_FAILURE;
        }

        // printf("wait P1, pid(P1) = %d\n", P1);
    }

    return EXIT_SUCCESS;
}