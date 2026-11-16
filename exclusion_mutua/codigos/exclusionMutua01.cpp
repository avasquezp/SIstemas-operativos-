/*
 * exclusionMutua01.cpp  --  LA MISMA CARRERA, SIN printf Y CON tope VARIABLE
 *
 * Qué hace:  Igual que exclusionMutua00 (5 hilos incrementan una variable
 *            compartida), pero ahora (a) el número de incrementos por hilo se
 *            pide por teclado y (b) el bucle no imprime nada.
 *
 * Para qué:  Sirve de "grupo de control". Demuestra que la condición de carrera
 *            no la provocaba el printf ni el valor fijo 5: el fallo está en el
 *            patrón leer-modificar-escribir sin protección, y con muchos más
 *            incrementos la pérdida se hace mucho más visible. Compara el
 *            resultado con exclusionMutua02, que hace lo mismo con un mutex y
 *            siempre da el total exacto.
 *
 * Compilar:  g++ exclusionMutua01.cpp -pthread -Wall -Wextra -o exclusionMutua01
 * Ejecutar:  ./exclusionMutua01      (prueba con topes como 1000, 50000, 1000000)
 */

#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>   /* isatty: distinguir entrada interactiva de una tubería */

#define NUM_FILOSOFOS 5
#define TOPE_POR_DEFECTO 5
/* Acotar el tope evita desbordar int: 5 hilos * tope no debe pasar de INT_MAX. */
#define TOPE_MAXIMO 1000000

void *filosofo(void *arg);

int variable = 0;
int tope = TOPE_POR_DEFECTO;

static void incrementar_sin_proteccion(void);

/* Lee el tope validando la entrada: si scanf falla o el valor no sirve se usa
 * el valor por defecto en vez de dejar `tope` con basura. */
static int leer_tope(void)
{
	int leido;

	printf("Ingrese tope : ");
	fflush(stdout);           /* sin newline, hay que forzar la salida */
	if (scanf("%d", &leido) != 1) {
		fprintf(stderr, "Entrada inválida; se usa tope = %d\n", TOPE_POR_DEFECTO);
		return TOPE_POR_DEFECTO;
	}
	/* Si la entrada no es un terminal (por ejemplo `echo 200000 | ./em01`), no
	 * existe el Enter que teclea el usuario y la siguiente salida quedaría
	 * pegada al mensaje; se añade el salto de línea que el terminal habría
	 * mostrado. Con entrada interactiva no se hace nada (el Enter ya lo pone). */
	if (!isatty(STDIN_FILENO)) {
		putchar('\n');
	}
	if (leido < 1 || leido > TOPE_MAXIMO) {
		fprintf(stderr, "tope fuera de rango [1, %d]; se usa %d\n",
		        TOPE_MAXIMO, TOPE_POR_DEFECTO);
		return TOPE_POR_DEFECTO;
	}
	return leido;
}

int main(void)
{
	pthread_t filosofos[NUM_FILOSOFOS];
	int identificadores[NUM_FILOSOFOS];
	int i, rc;
	int creados = 0;
	long esperado;

	tope = leer_tope();

	printf("variable = %d\n", variable);
	printf("tope = %d, valor esperado = %ld\n", tope,
	       (long)NUM_FILOSOFOS * (long)tope);

	/* `tope` se escribe aquí y ya no se modifica: cada hilo lo lee sin
	 * cerrojo y no hay carrera, porque pthread_create crea una relación
	 * happens-before entre lo que hizo main antes y lo que ve el hilo.
	 * (Si main lo cambiara mientras los hilos corren, sí habría carrera.) */
	for (i = 0; i < NUM_FILOSOFOS; i++) {
		identificadores[i] = i;
		rc = pthread_create(&filosofos[i], NULL, filosofo, &identificadores[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_create(%d): %s\n", i, strerror(rc));
			break;
		}
		creados++;
	}

	for (i = 0; i < creados; i++) {
		rc = pthread_join(filosofos[i], NULL);
		if (rc != 0) {
			fprintf(stderr, "pthread_join(%d): %s\n", i, strerror(rc));
		}
	}

	esperado = (long)NUM_FILOSOFOS * (long)tope;
	printf("variable = %ld (esperado %ld, incrementos perdidos %ld)\n",
	       (long)variable, esperado, esperado - (long)variable);

	if ((long)variable != esperado) {
		printf("=> FALLO: hubo condición de carrera (falta exclusión mutua).\n");
	} else {
		printf("=> Esta vez no se perdió ningún incremento (suerte, no corrección).\n");
	}

	return 0;
}

/* El mismo leer / ceder / escribir sin protección que exclusionMutua00, pero
 * aquí NO se ensancha la ventana con varias cesiones: se deja una sola y se
 * confía en el volumen (miles o millones de vueltas). Así se comprueba que la
 * carrera no era un truco del ejemplo de 00: con muchas repeticiones el número
 * de incrementos perdidos crece con el tope. */
static void incrementar_sin_proteccion(void)
{
	int valor_leido = variable;
	sched_yield();
	variable = valor_leido + 1;
}

void *filosofo(void *arg)
{
	int id = *((int *)arg);

	for (int i = 1; i <= tope; i++) {
		incrementar_sin_proteccion();
	}

	/* `id` no se usa para no ensuciar la salida con miles de líneas, pero se
	 * deja el parámetro para mantener la firma que exige pthread_create. */
	(void)id;
	return NULL;
}
