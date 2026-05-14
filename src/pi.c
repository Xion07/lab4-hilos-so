/*
 * pi.c — Versión serial del cálculo de π
 *
 * Utiliza integración numérica (regla del punto medio) sobre:
 *   ∫₀¹ 4 / (1 + x²) dx = π
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* GetTime()
 *   Retorna el tiempo actual en segundos con alta resolución.
 *   Usa CLOCK_MONOTONIC para medir intervalos sin saltos del reloj del sistema.
 */
double GetTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* f(x)
 *   Integrando del método: 4 / (1 + x²).
 *   Su integral definida en [0,1] converge a π.
 */
static inline double f(double x)
{
    return 4.0 / (1.0 + x * x);
}

/* CalcPi(n)
 *   Aproxima π dividiendo [0,1] en n rectángulos de ancho fH = 1/n.
 *   Evalúa f en el punto medio de cada rectángulo y acumula la suma.
 *   Retorna fH * Σ f(xᵢ).
 *
 *   Parámetros:
 *     n  — número de rectángulos (mayor n → mayor precisión).
 *   Retorna:
 *     Aproximación de π como valor double.
 */
double CalcPi(long n)
{
    const double fH  = 1.0 / (double)n;
    double       fSum = 0.0;
    double       fX;
    long         i;

    /* Bucle principal — núcleo computacional */
    for (i = 0; i < n; i++)
    {
        fX    = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    return fH * fSum;
}

/* main()
 *   Acepta n como argumento de línea de comandos.
 *   Mide el tiempo exclusivo de CalcPi e imprime resultado y métricas.
 */
int main(int argc, char *argv[])
{
    long n = 2000000000L;          /* valor por defecto: 2 × 10⁹ */

    if (argc > 1)
        n = atol(argv[1]);

    printf("Cálculo serial de π  —  n = %ld rectángulos\n", n);

    double t_ini = GetTime();
    double pi    = CalcPi(n);
    double t_fin = GetTime();

    printf("π ≈ %.15f\n", pi);
    printf("Error absoluto: %.2e\n", fabs(pi - M_PI));
    printf("Tiempo de ejecución (CalcPi): %.6f segundos\n", t_fin - t_ini);

    return 0;
}
