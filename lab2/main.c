/* Код программы для лабораторной работы №2 */
#include <math.h> // fabs()
#include <stdio.h> // io-functions
#include <sys/types.h> // pid_t
#include <unistd.h> // fork(), sleep(), usleep()
#include <stdlib.h> // fprintf(), fscanf()
#include <string.h>
#include <inttypes.h>

/* system cmd */
char *_touch = "touch ";
char *_rm = "rm ";
char *_2_dev_null = " 2> /dev/null";
/***************************************************/

/* ERROR CODE */
#define OK 0 // OK
#define DOUBLE_OVERFLOW 1 // Переполнение типа double 
#define INCORRECT_GRID_INPUT_DATA 2 // Некорретные данные для сетки 
#define FILE_OPEN_ERROR 3 // Файл не возможно открыть 
#define FILE_READ_ERROR 4 // Ошибка при чтении файла 
/***************************************************/

/* System data */
const char* InputFile = "input.txt"; // Файл входных данных для алгоритма расчета интеграла 
const char* InterProcessFileForCommunication1 = "InterProcess1.txt"; // Файл для межпроцессорного взаимодействия процесс 1
const char* InterProcessFileForCommunication2 = "InterProcess2.txt"; // Файл для межпроцессорного взаимодействия процесс 2
FILE _fd[2]; // Файловые дескрипторы
pid_t _pid1, _pid2; // Идентификаторы процессов 1 и 2
/****************************************************/

/* Math struct and var */
const double eps = 1e-15; // машинный ноль

/* 
  @details: Сетка для расчетной области шаг по области равномерный 
*/
struct _Grid
{
    int32_t K; // Количество шагов 
    double A; // Начало сетки
    double B;  // Конец 
    double step; // шаг на отрезке
};

/****************************************************/


/* MyMath */

/* Структура для представления функции */
struct _f_x
{
    double f1; // exp(x)
    double f2; // exp(-x)
    double f; // (f1-f2)/2 
}fx;

/*
    @param:
        double x
    @return: double
    @details: Расчет экспненты разложением в ряд тейлора
*/
double _exp(double x);
/*******************************************************/

/* System function */
/*
    @param: const char *filename - имя файла который хотим создать 
    @return : void
    @result: Создаем файл с заданным именем 
*/
void MakeFile(const char* filename);

/*
    @param: const char *filename - имя файла который хотим удалить 
    @return : void
    @result: Удаляем файл с заданным именем
*/
void DeleteFile(const char* filename);


/************************************************************/

/* Data preprocessing */
/*
    @param: const char *filename
    @param: struct _Grid *Grid - Структура сетки для инициализации
    @return: void
    @result: Валидация и загрузка данных из файла. При не удаче выход с кодом INCORRECT_GRID_INPUT_DATA
*/
void LoadData(const char *filename,struct _Grid *Grid);

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
    LoadData(InputFile, &Grid); // Загрузили данные 
    MakeFile(InterProcessFileForCommunication1); // Создали 1-ый файл для общения 
    MakeFile(InterProcessFileForCommunication2); // Создали 2-ой файл для общения
    
    /* Первый */
    if((_pid1 = fork()) == 0)
    {
        printf("Child 1: %d Parent = %d\n", getpid(), getppid());
        exit(OK);
    }

    /* Второй */
    if(_pid1 > 0 && (_pid2 = fork()) == 0)
    {
        printf("Child 2: %d Parent = %d\n", getpid(), getppid());
        exit(OK);
    }

    /* Родитель */
    
    printf("Parent: %d", getpid());

    /* Запускаем процессы */


    DeleteFile(InterProcessFileForCommunication1);
    DeleteFile(InterProcessFileForCommunication2);

    return 0;
}



void LoadData(const char *filename ,struct _Grid *Grid)
{
    FILE *fd = fopen(filename, "r");

    if(fd == NULL)
    {
        printf("Can not open file: %s", filename);
        exit(FILE_OPEN_ERROR);
    }

    if(fscanf(fd, "%lf %lf %d", &Grid->A, &Grid->B, &Grid->K) != 3)
    {
        fprintf(stderr, "Error read file: %s", filename);
        exit(FILE_READ_ERROR);
    }  

    /* Валидация полученных данных */
    if((Grid->A > Grid->B) || Grid->K <= 0)
    {
        fprintf(stderr,"Error input data from file: %s\n", filename);
        exit(INCORRECT_GRID_INPUT_DATA);
    }

    /* Данные корректные расчитываем шаг */
    Grid->step = (Grid->B - Grid->A)/(double)(Grid->K);

    close(fd);
}


void MakeFile(const char* filename)
{
    
    char *cmd = (char*)calloc(strlen(filename) + strlen(_touch) + strlen(_2_dev_null) + 1, sizeof(char));
    /* create cmd */
    memmove(cmd, _touch, strlen(_touch));
    strcat(cmd, filename);
    strcat(cmd, _2_dev_null);
    /* execute */
    system(cmd);

    free(cmd);
}


void DeleteFile(const char* filename)
{
    char *cmd = (char*)calloc(strlen(filename) + strlen(_rm) + strlen(_2_dev_null) + 1, sizeof(char));
    
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

    double e = 1.0; // результат расчета
    double ei = 1.0; // i-ая итерация
    
    double tmpx = -1.0*fabs(x); // -1*|x| - для расчета функции e^-x для него погрешность легко оценить 
    /* Расчет для функции e^-x */
    for(int n = 1; n < 1e4; n++)
    {   
        ei = (ei*tmpx)/(double)n;
        e += ei;
        if(fabs(ei) < eps) break;
    }

    /* Возврат значения с учетом знака */
    if(isinf(e) || isnan(e))
    {
        fprintf(stderr, "double overflow or incorrect math operation in function _exp() with arg = %lf", x);
        exit(DOUBLE_OVERFLOW);
    }
    if(x > eps) return 1.0/e;
    else return e;
}

void PrintGrid(const struct _Grid *Grid)
{
    printf("Grid data\n");
    printf("A = %lf B = %lf  Step = %lf  K = %u", Grid->A, Grid->B, Grid->step, Grid->K);
}