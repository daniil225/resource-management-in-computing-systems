/* Код программы для лабораторной работы №2 */
#include <math.h>      // fabs()
#include <stdio.h>     // io-functions
#include <sys/types.h> // pid_t
#include <unistd.h>    // fork(), sleep(), usleep()
#include <stdlib.h>    // fprintf(), fscanf()
#include <string.h>
#include <inttypes.h>
#include <sys/wait.h>

/* system cmd */
char *_touch = "touch ";
char *_rm = "rm ";
char *_2_dev_null = " 2> /dev/null";
/***************************************************/

/* ERROR CODE */
#define OK 0                        // OK
#define DOUBLE_OVERFLOW 1           // Переполнение типа double
#define INCORRECT_GRID_INPUT_DATA 2 // Некорретные данные для сетки
#define FILE_OPEN_ERROR 3           // Файл не возможно открыть
#define FILE_READ_ERROR 4           // Ошибка при чтении файла
#define FUNCTION_CALCULATION_ERROR 5 // Ошибка при расчете функции f(x)
/***************************************************/

/* System data */
const char *InputFile = "input.txt";                                 // Файл входных данных для алгоритма расчета интеграла
const char *InterProcessFileForCommunication1 = "InterProcess1.txt"; // Файл для межпроцессорного взаимодействия процесс 1
const char *InterProcessFileForCommunication2 = "InterProcess2.txt"; // Файл для межпроцессорного взаимодействия процесс 2
FILE *_fd[2];                                                        // Файловые дескрипторы
pid_t _pid1, _pid2;                                                  // Идентификаторы процессов 1 и 2
/****************************************************/

/* Math struct and var */
const double eps = 1e-15; // машинный ноль

/*
  @details: Сетка для расчетной области шаг по области равномерный
*/
struct _Grid
{
    int32_t K;   // Количество шагов
    double A;    // Начало сетки
    double B;    // Конец
    double step; // шаг на отрезке
};

/****************************************************/

/* MyMath */

/* Структура для представления функции */
struct _f_x
{
    pid_t pid1; // Номер первого процесса
    double f1;  // exp(x)

    pid_t pid2; // Номер 2-ого процесса
    double f2;  // exp(-x)
    double f;   // (f1-f2)/2
} fx;

/*
    @param:
        double x
    @return: double
    @details: Расчет экспненты разложением в ряд тейлора
*/
double _exp(double x);

/*
    @param: double x
    @return: void
    @result: Запускает 2 процесса расчитывает функцию и в конце объединяет результат в одну общую функцию
             При этом в поле fx.f будет результат расчета целевой функции 
             Обработка ошибок происходит внутри в случае ошибки выход с соответсвующим кодом 
 */
void CalcFunction_fx(double x);

/*
    @param: const struct _Grid *Grid - сетка
    @result: double - результат расчета интеграла 
    @result: -
*/
double IntegrateTrapetcoid(const struct _Grid *Grid);
/*******************************************************/

/* System function */

/*
    @param: const char *filename - имя файла который хотим создать
    @return : void
    @result: Создаем файл с заданным именем
*/
void MakeFile(const char *filename);

/*
    @param: const char *filename - имя файла который хотим удалить
    @return : void
    @result: Удаляем файл с заданным именем
*/
void DeleteFile(const char *filename);

/************************************************************/

/* Data preprocessing */
/*
    @param: const char *filename
    @param: struct _Grid *Grid - Структура сетки для инициализации
    @return: void
    @result: Валидация и загрузка данных из файла. При не удаче выход с кодом INCORRECT_GRID_INPUT_DATA
*/
void LoadData(const char *filename, struct _Grid *Grid);

/************************************************************/

/* Debug functions */
/*
    @param: const struct _Grid *Grid
    @return: void
    @result: -
*/
void PrintGrid(const struct _Grid *Grid);

/************************************************************/

int main()
{
    struct _Grid Grid;
    LoadData(InputFile, &Grid);                  // Загрузили данные
    MakeFile(InterProcessFileForCommunication1); // Создали 1-ый файл для общения
    MakeFile(InterProcessFileForCommunication2); // Создали 2-ой файл для общения

    double integ_res = IntegrateTrapetcoid(&Grid);
    printf("res = %lf", integ_res);

    DeleteFile(InterProcessFileForCommunication1);
    DeleteFile(InterProcessFileForCommunication2);

    return 0;
}

double IntegrateTrapetcoid(const struct _Grid *Grid)
{
    CalcFunction_fx(Grid->A);
    double f0 = fx.f;
    CalcFunction_fx(Grid->B);
    double fn = fx.f;
    double res = (f0+fn)/2.0;

    for(int32_t i = 1; i < Grid->K; i++)
    {
        double x = Grid->A + (double)(i)*Grid->step;
        CalcFunction_fx(x);
        res += fx.f;
    }

    return res*Grid->step;
}

void CalcFunction_fx(double x)
{
    /* Первый - расчет exp(x) */
    if ((_pid1 = fork()) == 0)
    {
        // открываем файл на запись
        _fd[0] = fopen(InterProcessFileForCommunication1, "w");

        if (_fd[0] == NULL)
        {
            fprintf(stderr, "can not open file : %s ", InterProcessFileForCommunication1);
            exit(FILE_OPEN_ERROR);
        }
        // printf("Child 1: %d Parent = %d\n", getpid(), getppid());

        uint32_t pid = getpid();
        double res_f = _exp(x);
        fprintf(_fd[0], "%d %lf", pid, res_f);
        fclose(_fd[0]);
        exit(OK);
    }

    /* Второй - расчет exp(-x) */
    else if (_pid1 > 0 && (_pid2 = fork()) == 0)
    {
        // открываем файл на запись
        _fd[1] = fopen(InterProcessFileForCommunication2, "w");
        if (_fd[1] == NULL)
        {
            fprintf(stderr, "can not open file : %s ", InterProcessFileForCommunication2);
            exit(FILE_OPEN_ERROR);
        }
        // printf("Child 2: %d Parent = %d\n", getpid(), getppid());
        uint32_t pid = getpid();
        double res_f = _exp(-x);
        fprintf(_fd[1], "%d %lf", pid, res_f);
        fclose(_fd[1]);
        exit(OK);
    }
    else
    {
        /* Родитель */
        int status1 = 0;
        int status2 = 0;
        waitpid(_pid1, &status1, NULL); // Ждем первого и его код возврата
        waitpid(_pid2, &status2, NULL); // Ждем второго и его код возврата

        if (status1 == OK)
        {
            _fd[0] = fopen(InterProcessFileForCommunication1, "r");
            if (_fd[0] == NULL)
            {
                fprintf(stderr, "Can not open file: %s", InterProcessFileForCommunication1);
                exit(FILE_OPEN_ERROR);
            }

            fscanf(_fd[0], "%d %lf", &fx.pid1, &fx.f1);
            fclose(_fd[0]);
        }
        else
        {
            fprintf(stderr, "Error function calc status = %d", status1);
            exit(FUNCTION_CALCULATION_ERROR);
        }

        if (status2 == OK)
        {
            _fd[1] = fopen(InterProcessFileForCommunication2, "r");
            // Аналогично верхнему
            if (_fd[1] == NULL)
            {
                fprintf(stderr, "Can not open file: %s", InterProcessFileForCommunication2);
                exit(FILE_OPEN_ERROR);
            }
            fscanf(_fd[1], "%d %lf", &fx.pid2, &fx.f2);
            fclose(_fd[1]);
        }
        else
        {
            fprintf(stderr, "Error function calc status = %d", status2);
            exit(FUNCTION_CALCULATION_ERROR);
        }

        /* Объединение результата */
        fx.f = (fx.f1 - fx.f2)/2.0;
    }
}

void LoadData(const char *filename, struct _Grid *Grid)
{
    FILE *fd = fopen(filename, "r");

    if (fd == NULL)
    {
        fprintf(stderr, "Can not open file: %s", filename);
        exit(FILE_OPEN_ERROR);
    }

    if (fscanf(fd, "%lf %lf %d", &Grid->A, &Grid->B, &Grid->K) != 3)
    {
        fprintf(stderr, "Error read file: %s", filename);
        exit(FILE_READ_ERROR);
    }

    /* Валидация полученных данных */
    if ((Grid->A > Grid->B) || Grid->K <= 0)
    {
        fprintf(stderr, "Error input data from file: %s\n", filename);
        exit(INCORRECT_GRID_INPUT_DATA);
    }

    /* Данные корректные расчитываем шаг */
    Grid->step = (Grid->B - Grid->A) / (double)(Grid->K);

    close(fd);
}

void MakeFile(const char *filename)
{

    char *cmd = (char *)calloc(strlen(filename) + strlen(_touch) + strlen(_2_dev_null) + 1, sizeof(char));
    /* create cmd */
    memmove(cmd, _touch, strlen(_touch));
    strcat(cmd, filename);
    strcat(cmd, _2_dev_null);
    /* execute */
    system(cmd);

    free(cmd);
}

void DeleteFile(const char *filename)
{
    char *cmd = (char *)calloc(strlen(filename) + strlen(_rm) + strlen(_2_dev_null) + 1, sizeof(char));

    /* create cmd */
    memmove(cmd, _rm, strlen(_rm));
    strcat(cmd, filename);
    strcat(cmd, _2_dev_null);
    /* execute */
    system(cmd);

    free(cmd);
}

double _exp(double x)
{

    int n = 1; // Счетчик

    double e = 1.0;  // результат расчета
    double ei = 1.0; // i-ая итерация

    double tmpx = -1.0 * fabs(x); // -1*|x| - для расчета функции e^-x для него погрешность легко оценить
    /* Расчет для функции e^-x */
    for (int n = 1; n < 1e4; n++)
    {
        ei = (ei * tmpx) / (double)n;
        e += ei;
        if (fabs(ei) < eps)
            break;
    }

    /* Возврат значения с учетом знака */
    if (isinf(e) || isnan(e))
    {
        fprintf(stderr, "double overflow or incorrect math operation in function _exp() with arg = %lf", x);
        exit(DOUBLE_OVERFLOW);
    }
    if (x > eps)
        return 1.0 / e;
    else
        return e;
}

void PrintGrid(const struct _Grid *Grid)
{
    printf("Grid data\n");
    printf("A = %lf B = %lf  Step = %lf  K = %u", Grid->A, Grid->B, Grid->step, Grid->K);
}