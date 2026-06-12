#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Función auxiliar para obtener tiempo en segundos */
double GetTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* Función base para la integración numérica */
double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

/* Función CalcPi: calcula π usando integración numérica */
double CalcPi(int n)
{
    const double fH = 1.0 / (double)n;
    double fSum = 0.0;
    double fX;
    int i;

    /* Bucle principal para calcular la suma */
    for (i = 0; i < n; i += 1)
    {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    return fH * fSum;
}

int main(int argc, char *argv[])
{
    int n;
    double pi;
    double start, end;

    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <número de rectángulos>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);

    if (n <= 0)
    {
        fprintf(stderr, "Error: n debe ser un número positivo\n");
        return 1;
    }

    /* Medir tiempo de ejecución */
    start = GetTime();
    pi = CalcPi(n);
    end = GetTime();

    printf("Valor aproximado de π: %.15f\n", pi);
    printf("Valor real de π:       %.15f\n", M_PI);
    printf("Error:                 %.15f\n", fabs(pi - M_PI));
    printf("Tiempo de ejecución:   %.6f segundos\n", end - start);

    return 0;
}
