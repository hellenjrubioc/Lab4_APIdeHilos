#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Estructura para pasar argumentos al hilo trabajador */
typedef struct
{
    long *array;
    int n;
} fib_args_t;

/* Función de trabajo del hilo: genera la secuencia de Fibonacci */
void *FibonacciWorker(void *arg)
{
    fib_args_t *args = (fib_args_t *)arg;
    long *array = args->array;
    int n = args->n;
    int i;

    /* Generar los primeros n números de Fibonacci */
    if (n >= 1)
    {
        array[0] = 0;
    }
    if (n >= 2)
    {
        array[1] = 1;
    }

    /* Generar el resto de la secuencia */
    for (i = 2; i < n; i++)
    {
        array[i] = array[i - 1] + array[i - 2];
    }

    /* El hilo termina su ejecución */
    return NULL;
}

int main(int argc, char *argv[])
{
    int n;
    long *fib_array;
    pthread_t worker_thread;
    fib_args_t thread_args;
    int i;

    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <número de elementos>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);

    if (n <= 0)
    {
        fprintf(stderr, "Error: n debe ser un número positivo\n");
        return 1;
    }

    /* Asignar memoria dinámica para el arreglo compartido */
    fib_array = (long *)malloc(n * sizeof(long));

    if (!fib_array)
    {
        fprintf(stderr, "Error: no se pudo asignar memoria\n");
        return 1;
    }

    /* Preparar los argumentos para el hilo trabajador */
    thread_args.array = fib_array;
    thread_args.n = n;

    /* Crear el hilo trabajador */
    if (pthread_create(&worker_thread, NULL, FibonacciWorker, &thread_args) != 0)
    {
        fprintf(stderr, "Error: no se pudo crear el hilo trabajador\n");
        free(fib_array);
        return 1;
    }

    /* El hilo principal se bloquea esperando que el trabajador termine */
    if (pthread_join(worker_thread, NULL) != 0)
    {
        fprintf(stderr, "Error: no se pudo sincronizar con el hilo trabajador\n");
        free(fib_array);
        return 1;
    }

    /* Después de pthread_join, el hilo principal imprime la secuencia */
    printf("Secuencia de Fibonacci (%d elementos):\n", n);
    for (i = 0; i < n; i++)
    {
        if (i > 0 && i % 10 == 0)
        {
            printf("\n");
        }
        printf("%ld ", fib_array[i]);
    }
    printf("\n");

    /* Liberar memoria */
    free(fib_array);

    return 0;
}
