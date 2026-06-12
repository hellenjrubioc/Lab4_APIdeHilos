#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

/* Estructura para pasar argumentos a los hilos */
typedef struct
{
    int thread_id;
    int num_threads;
    int n;
    double partial_sum;
} thread_args_t;

/* Variables globales */
const double fH_global;
int n_global;
int num_threads_global;

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

/* Función de trabajo del hilo: calcula la suma parcial */
void *ThreadCalcPi(void *arg)
{
    thread_args_t *args = (thread_args_t *)arg;
    int thread_id = args->thread_id;
    int num_threads = args->num_threads;
    int n = args->n;
    
    const double fH = 1.0 / (double)n;
    double fSum = 0.0;
    double fX;
    int i;

    /* Calcular el rango de iteraciones para este hilo */
    int iterations_per_thread = n / num_threads;
    int start = thread_id * iterations_per_thread;
    int end = (thread_id == num_threads - 1) ? n : (thread_id + 1) * iterations_per_thread;

    /* Calcular suma parcial para este hilo */
    for (i = start; i < end; i++)
    {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }

    /* Guardar el resultado parcial */
    args->partial_sum = fH * fSum;

    return NULL;
}

int main(int argc, char *argv[])
{
    int n, num_threads;
    double pi = 0.0;
    double start, end;
    pthread_t *threads;
    thread_args_t *thread_data;
    int i;

    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <número de rectángulos> <número de hilos>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    num_threads = atoi(argv[2]);

    if (n <= 0 || num_threads <= 0)
    {
        fprintf(stderr, "Error: n y num_threads deben ser números positivos\n");
        return 1;
    }

    /* Asignar memoria para los hilos y datos */
    threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_data = (thread_args_t *)malloc(num_threads * sizeof(thread_args_t));

    if (!threads || !thread_data)
    {
        fprintf(stderr, "Error: no se pudo asignar memoria\n");
        return 1;
    }

    /* Inicializar argumentos para cada hilo */
    for (i = 0; i < num_threads; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].n = n;
        thread_data[i].partial_sum = 0.0;
    }

    /* Medir tiempo de ejecución */
    start = GetTime();

    /* Crear los hilos */
    for (i = 0; i < num_threads; i++)
    {
        if (pthread_create(&threads[i], NULL, ThreadCalcPi, &thread_data[i]) != 0)
        {
            fprintf(stderr, "Error: no se pudo crear el hilo %d\n", i);
            return 1;
        }
    }

    /* Esperar a que todos los hilos terminen y agregar resultados */
    for (i = 0; i < num_threads; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            fprintf(stderr, "Error: no se pudo sincronizar el hilo %d\n", i);
            return 1;
        }
        pi += thread_data[i].partial_sum;
    }

    end = GetTime();

    printf("Valor aproximado de π: %.15f\n", pi);
    printf("Valor real de π:       %.15f\n", M_PI);
    printf("Error:                 %.15f\n", fabs(pi - M_PI));
    printf("Tiempo de ejecución:   %.6f segundos\n", end - start);
    printf("Número de hilos:       %d\n", num_threads);

    /* Liberar memoria */
    free(threads);
    free(thread_data);

    return 0;
}
