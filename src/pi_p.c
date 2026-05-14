/*
 * pi_p.c — Versión paralela del cálculo de π usando Pthreads
 *
 * Estrategia: Data Parallelism.
 *   El rango [0, n-1] se particiona entre T hilos. Cada hilo calcula
 *   una suma parcial en una variable local (sin mutex en el bucle) y
 *   la retorna al hilo principal mediante pthread_exit / pthread_join.
 *   El hilo main agrega todas las sumas parciales y multiplica por fH.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

/* GetTime()
 *   Retorna el tiempo actual en segundos.
 */
double GetTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* f(x)
 *   Integrando: 4 / (1 + x²).
 */
static inline double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

/* ThreadArgs — estructura de argumentos para cada hilo trabajador.
 *
 *   Campos:
 *     start — índice de inicio del sub-rango (inclusive).
 *     end   — índice de fin del sub-rango (exclusive).
 *     fH    — ancho de cada rectángulo (= 1/n), compartido de solo lectura.
 */
typedef struct {
    long   start;
    long   end;
    double fH;
} ThreadArgs;

/* calcPiThread(arg)
 *   Función ejecutada por cada hilo trabajador.
 *
 *   Recibe un puntero a ThreadArgs y calcula la suma parcial del
 *   sub-rango [start, end). Almacena el resultado en memoria dinámica
 *   y lo retorna mediante pthread_exit para que main lo recoja con
 *   pthread_join.
 *
 *   Parámetros:
 *     arg — puntero a ThreadArgs con el sub-rango asignado.
 *   Retorna (vía pthread_exit):
 *     Puntero a double con la suma parcial.
 */
void *calcPiThread(void *arg)
{
    ThreadArgs *tArgs    = (ThreadArgs *)arg;
    double      localSum = 0.0;
    double      fX;
    long        i;

    /* Bucle del sub-rango — sin mutex, todo en variable local */
    for (i = tArgs->start; i < tArgs->end; i++)
    {
        fX       = tArgs->fH * ((double)i + 0.5);
        localSum += f(fX);
    }

    /* Alojar resultado en heap para retornarlo de forma segura */
    double *result = (double *)malloc(sizeof(double));
    if (!result) {
        fprintf(stderr, "Error: malloc falló en hilo.\n");
        pthread_exit(NULL);
    }
    *result = localSum;
    pthread_exit((void *)result);
}

/* main(argc, argv)
 *   Orquesta la creación de T hilos, espera su finalización con
 *   pthread_join, agrega las sumas parciales y calcula π.
 *
 *   Argumentos de línea de comandos:
 *     argv[1] — n: número total de rectángulos.
 *     argv[2] — T: número de hilos a crear.
 */
int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <n> <T>\n", argv[0]);
        fprintf(stderr, "  n = número de rectángulos\n");
        fprintf(stderr, "  T = número de hilos\n");
        return EXIT_FAILURE;
    }

    long n = atol(argv[1]);
    int  T = atoi(argv[2]);

    if (n <= 0 || T <= 0) {
        fprintf(stderr, "Error: n y T deben ser enteros positivos.\n");
        return EXIT_FAILURE;
    }

    printf("Cálculo paralelo de π  —  n = %ld  |  T = %d hilos\n", n, T);

    /* Reservar arreglos de IDs de hilo y argumentos */
    pthread_t  *threads = (pthread_t  *)malloc(T * sizeof(pthread_t));
    ThreadArgs *args    = (ThreadArgs *)malloc(T * sizeof(ThreadArgs));
    if (!threads || !args) {
        fprintf(stderr, "Error: malloc falló en main.\n");
        return EXIT_FAILURE;
    }

    double fH        = 1.0 / (double)n;
    long   chunkSize = n / T;          /* iteraciones por hilo (base) */
    long   remainder = n % T;         /* iteraciones sobrantes */

    /* Inicio de la región cronometrada */
    double t_ini = GetTime();

    /* Crear T hilos, asignando sub-rangos contiguos */
    long offset = 0;
    for (int i = 0; i < T; i++)
    {
        /* Distribuir el remainder una iteración extra a los primeros hilos */
        long myChunk     = chunkSize + (i < remainder ? 1 : 0);
        args[i].start    = offset;
        args[i].end      = offset + myChunk;
        args[i].fH       = fH;
        offset          += myChunk;

        if (pthread_create(&threads[i], NULL, calcPiThread, &args[i]) != 0) {
            fprintf(stderr, "Error al crear hilo %d\n", i);
            return EXIT_FAILURE;
        }
    }

    /* Recolectar resultados parciales con pthread_join */
    double totalSum = 0.0;
    for (int i = 0; i < T; i++)
    {
        double *partial = NULL;
        pthread_join(threads[i], (void **)&partial);
        if (partial) {
            totalSum += *partial;
            free(partial);
        }
    }

    double pi    = fH * totalSum;
    double t_fin = GetTime();
    /* Fin de la región cronometrada */

    printf("π ≈ %.15f\n", pi);
    printf("Error absoluto: %.2e\n", fabs(pi - M_PI));
    printf("Tiempo de ejecución (CalcPi paralelo): %.6f segundos\n", t_fin - t_ini);

    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
