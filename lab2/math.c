#include <math.h>
#include <stdio.h>

const double eps = 1e-15; // машинный ноль

/*
    @param:
        double x
    @return: double
    @details: Расчет экспненты разложением в ряд тейлора
*/
double _exp(double x);

int main()
{
    double arg = 3.0;
    double myval = _exp(arg);
    double val = exp(arg);

    printf("myexp(%lf) = %lf exp(%lf) = %lf diff = %.13lf", arg, myval, arg, val, myval - val);
    
    
    return 0;
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
    if(x > eps) return 1.0/e;
    else return e;
}