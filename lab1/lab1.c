#include <dirent.h> // Структура папки
#include <stdio.h>  // Стандартный ввод/вывод
#include <stdlib.h> // Стандартная библиотека
#include <string.h> // Строки
#include <errno.h>  // Проверка существования папки
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define OK 0            //
#define NO_ARG_ERR 1    // No input parameters specified
#define DIRECTORY_ERR 2 // Incorrectly specified directory
#define MANY_ARG_ERR 3  // Many argument

#define false 0
#define true 1
typedef int32_t bool;

/*
    @param: const char* str - строка
    @return: int32_t - 1 - стока "." или ".." 0 - в противном случае
*/
int32_t isSystemDir(const char *str);

/*
    @param: char *pathDir - Путь до каталога
    @return: void
    @result: Печть самого вложенного каталога
*/
void printTheInternalCatalog(char *pathDir);

/*
    @param:
        int32_t argc - количество аргументов
        char **argv - список аргументов командной строки
    @return: void
    @result: Проверка входных параметров и вывод соответсвующих сообщение об ошибках или действий для параметра  --help
*/

/*
    @param: const char* __name - имя каталога
    @return: DIR* - дескриптор открытого каталога
    @result: Безопасное открытие и соответсвующая обработка ошибок
*/
DIR *openDir(const char *__name, bool checkIsDir);

/*
    @param: DIR *__dirp
    @return: void
    @result: Закрытие директории, если не удачно то выход с соответсвующим кодом
*/
void closeDir(DIR *__dirp);

void checkInputArgument(int32_t argc, char **argv);

int32_t main(int32_t argc, char **argv)
{

    checkInputArgument(argc, argv);
    char *path = argv[1];

    int32_t DirCount = 0;
    int32_t AnyFile = 0;                          // Счетчик каких либо файлов в директории
    DIR *dir = openDir((const char *)path, true); // Открываем переданный каталог

    struct dirent *entry;

    while (entry = readdir(dir))
    {
        if (isSystemDir(entry->d_name) == 0 && entry->d_type == 4)
        {
            char *pathDir = (char *)calloc((strlen(path) + 1 + strlen(entry->d_name) + 1), sizeof(char)); // +2 так как strlen - не учитывает \0
            memmove(pathDir, path, strlen(path));                                                         // гарантирует корректное поведение для строк
            strcat(pathDir, "/");                                                                         // в конец помещется \0
            strcat(pathDir, entry->d_name);                                                               // в конец помещется \0
            printTheInternalCatalog(pathDir);
            free(pathDir); // Переменная локальная тоже не совсем нужно
            DirCount++;
        }
        if (isSystemDir(entry->d_name) == 0)
            AnyFile++;
    }

    closeDir(dir);

    // Если ни одного подкаталога в каталоге не обнаружено печатаем имя этого каталога
    if (AnyFile == 0 || DirCount == 0)
        printf(" is empty folder!");
    return 0;
}

void checkInputArgument(int32_t argc, char **argv)
{
    if (argc > 2)
    {
        fprintf(stderr,"Many argument pass, you must pass 1 arg: dirname\n");
        exit(MANY_ARG_ERR);
    }
    else if (argc < 2)
    {
        fprintf(stderr,"directory not specified!\n");
        exit(NO_ARG_ERR);
    }
    else if (strcmp(argv[1], "-help") == 0)
    {
        printf("Instruction:\n");
        printf("1) Enter the name of the directory for which you want to display folders that do not contain subdirectories.\n");
        printf("2) If the directory you entered does not exist, an appropriate error message will be displayed.\n");
        printf("3) If an incorrect number of parameters is entered (not equal to 1), an error message is displayed.\n");
        exit(OK);
    }

    return;
}

DIR *openDir(const char *__name, bool checkIsDir)
{
    bool isDir = true;
    DIR *dir = NULL;

    if (checkIsDir == true)
    {
        struct stat statBuf;
        if (stat(__name, &statBuf) == 0)
        {
            if (!S_ISDIR(statBuf.st_mode)) isDir = false;
        }
        else if (errno == ENOENT)
        {
            fprintf(stderr,"directory not specified!\n");
            exit(DIRECTORY_ERR);
        }
    }

    if (isDir == true)
    {
        dir = opendir(__name); // Открываем переданный каталог
                               // printf("try open: %s, size: %ld\n", __name, strlen(__name));
        if (errno == ENOENT)
        {
            fprintf(stderr,"Directory not found. Open error\n");
            exit(DIRECTORY_ERR);
        }
        else if (errno == ENOMEM)
        {
           fprintf(stderr,"Memory error. Open error\n");
            exit(DIRECTORY_ERR);
        }
    }
    else
    {
        fprintf(stderr, "%s is not a directory!\n", __name);
        exit(DIRECTORY_ERR);
    }

    return dir;
}

void closeDir(DIR *__dirp)
{
    // Закрытие каталога
    // printf("Close Dir: %p\n", __dirp);
    if (closedir(__dirp) != 0)
    {
        perror("Некорректное закрытие каталога");
        exit(DIRECTORY_ERR);
    }
    return;
}

int32_t isSystemDir(const char *str)
{
    int32_t res = 0;
    if (strcmp(str, ".") == 0)
        res = 1;
    if (strcmp(str, "..") == 0)
        res = 1;
    return res;
}

void printTheInternalCatalog(char *pathDir)
{
    int32_t DirCount = 0; // Счетчик каталогов в папке
    // Open dir
    DIR *dir = openDir((const char *)pathDir, false);

    struct dirent *entry;

    while (entry = readdir(dir))
    {
        if (isSystemDir(entry->d_name) == 0 && entry->d_type == 4)
        {
            DirCount++; // Увеличиваем количество счетчика каталогов в вызове
            // Формируем путь к каталогу
            char *newPath = (char *)calloc(strlen(pathDir) + 1 + strlen(entry->d_name) + 1, sizeof(char)); // +2 так как strlen - не учитывает \0
            memmove(newPath, pathDir, strlen(pathDir));                                                    // гарантирует корректное поведение для строк
            strcat(newPath, "/");                                                                          // в конец помещется \0
            strcat(newPath, entry->d_name);                                                                // в конец помещется \0
            printTheInternalCatalog(newPath);
            free(newPath); // Вообще не совсем нужно так как переменная локальная
        }
    }

    // Закрытие каталога
    closeDir(dir);

    // Если ни одного подкаталога в каталоге не обнаружено печатаем имя этого каталога
    if (DirCount == 0)
        printf("%s\n", pathDir);
    // free(pathDir);

    return;
}