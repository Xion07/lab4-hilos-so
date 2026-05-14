/*
 * fibonacci.c — Generador de la secuencia de Fibonacci con Pthreads
 *
 * Diseño:
 *   1. main asigna dinámicamente un arreglo compartido de tamaño N.
 *   2. main crea UN hilo trabajador, pasándole un puntero al arreglo y N.
 *   3. El hilo trabajador llena el arreglo con los N primeros valores
 *      de la secuencia de Fibonacci de forma iterativa.
 *   4. main se bloquea con pthread_join hasta que el trabajador termine.
 *   5. Tras pthread_join, main imprime el arreglo ya completo.
 *
 * Secuencia: F(0)=0, F(1)=1, F(i)=F(i-1)+F(i-2)  para i ≥ 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/* GetTime()
 *   Retorna el tiempo actual en segundos.
 *   Usada para medir el tiempo de ejecución del hilo trabajador.
 */
double GetTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* FibArgs — estructura de argumentos para el hilo trabajador.
 *
 *   Campos:
 *     array — puntero al arreglo compartido.
 *     N     — cantidad de elementos a calcular.
 */
typedef struct {
    long long *array;
    int        N;
} FibArgs;

/* fibWorker(arg)
 *   Función ejecutada por el hilo trabajador.
 *
 *   Calcula iterativamente los N primeros números de Fibonacci y los
 *   almacena en el arreglo compartido.
 *
 *   Parámetros:
 *     arg — puntero a FibArgs con el arreglo y el tamaño N.
 *   Retorna:
 *     NULL (los resultados quedan en el arreglo compartido).
 */
void *fibWorker(void *arg)
{
    FibArgs   *fArgs = (FibArgs *)arg;
    long long *fib   = fArgs->array;
    int        N     = fArgs->N;

    if (N >= 1) fib[0] = 0LL;
    if (N >= 2) fib[1] = 1LL;

    for (int i = 2; i < N; i++)
        fib[i] = fib[i - 1] + fib[i - 2];

    pthread_exit(NULL);
}

/* fibSerial(N, array)
 *
 *   Calcula la secuencia de Fibonacci de forma serial (sin hilos).
 *   Se usa en el análisis comparativo del notebook para N grande.
 *
 *   Parámetros:
 *     N     — cantidad de elementos.
 *     array — arreglo de salida (debe estar asignado por el llamador).
 */
void fibSerial(int N, long long *array)
{
    if (N >= 1) array[0] = 0LL;
    if (N >= 2) array[1] = 1LL;
    for (int i = 2; i < N; i++)
        array[i] = array[i - 1] + array[i - 2];
}

/* main(argc, argv)
 *   Punto de entrada. Gestiona la memoria, crea el hilo trabajador y
 *   espera su finalización antes de imprimir la secuencia.
 *
 *   Argumentos de línea de comandos:
 *     argv[1] — N: cantidad de elementos de Fibonacci a generar.
 */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <N>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "Error: N debe ser un entero positivo.\n");
        return EXIT_FAILURE;
    }

    /* Paso 1: main asigna el arreglo compartido */
    long long *fibArray = (long long *)malloc(N * sizeof(long long));
    if (!fibArray) {
        fprintf(stderr, "Error: malloc falló.\n");
        return EXIT_FAILURE;
    }

    /* Paso 2: preparar argumentos y crear el hilo trabajador */
    FibArgs args = { fibArray, N };
    pthread_t tid;

    double t_ini = GetTime();

    if (pthread_create(&tid, NULL, fibWorker, &args) != 0) {
        fprintf(stderr, "Error al crear el hilo trabajador.\n");
        free(fibArray);
        return EXIT_FAILURE;
    }

    /* Paso 3: main se bloquea hasta que el trabajador termine */
    pthread_join(tid, NULL);

    double t_fin = GetTime();

    /* Paso 4: imprimir la secuencia (arreglo ya completo) */
    printf("Secuencia de Fibonacci — %d elementos\n", N);
    printf("%-6s  %s\n", "Índice", "F(i)");
    printf("%-6s  %s\n", "------", "--------------------");
    for (int i = 0; i < N; i++)
        printf("F(%-4d) = %lld\n", i, fibArray[i]);

    printf("\nTiempo de cálculo (hilo trabajador): %.6f segundos\n",
           t_fin - t_ini);

    free(fibArray);
    return EXIT_SUCCESS;
}
