/*
 * exclusionMutua00.cpp  --  VERSIÓN SIN EXCLUSIÓN MUTUA (condición de carrera)
 *
 * Qué hace:  5 hilos ("filósofos") incrementan 5 veces cada uno una variable
 *            global compartida. Al final la variable debería valer 25, pero
 *            casi nunca vale 25: cada ejecución da un número distinto.
 *
 * Por qué:   `variable = variable + 1` NO es una operación atómica. El
 *            procesador la ejecuta en tres pasos (leer, sumar, escribir) y el
 *            sistema operativo puede quitarle la CPU al hilo entre uno y otro.
 *            Si dos hilos leen el mismo valor y ambos escriben "leído + 1", los
 *            dos escriben lo mismo y uno de los dos incrementos se pierde.
 *            Esto se llama condición de carrera (data race) y es comportamiento
 *            indefinido en C++: el resultado no está garantizado por el estándar.
 *
 * Compilar:  g++ exclusionMutua00.cpp -pthread -Wall -Wextra -o exclusionMutua00
 *            (sin -O2: al optimizar, el compilador puede cachear `variable` en un
 *             registro y el ejemplo deja de reproducirse... eso ya es el síntoma
 *             del data race, no una solución; la solución está en exclusionMutua02)
 *
 * Ejecutar:  ./exclusionMutua00
 */

#include <stdio.h>
#include <pthread.h>
#include <sched.h>     /* sched_yield(): ceder la CPU voluntariamente */
#include <string.h>

#define NUM_FILOSOFOS 5   /* hilos que compiten */
#define ITERACIONES   5   /* incrementos por hilo -> total esperado 25 */

void *filosofo(void *arg);

/* ------------------------------------------------------------------ *
 * RECURSO COMPARTIDO
 * La variable vive en el segmento de datos y la ven todos los hilos.
 * Nadie la protege: este es exactamente el error que el programa quiere
 * mostrar. En exclusionMutua02 se protege con un mutex.
 * ------------------------------------------------------------------ */
int variable = 0;

int main(void)
{
	pthread_t filosofos[NUM_FILOSOFOS];
	int identificadores[NUM_FILOSOFOS];   /* un hueco por hilo, no compartido */
	int i, rc;
	int creados = 0;
	int esperado = NUM_FILOSOFOS * ITERACIONES;

	printf("variable = %d\n", variable);
	printf("Valor esperado al final: %d\n", esperado);

	/* Se crean los hilos. Cada uno recibe la dirección de SU propio entero:
	 * si todos recibieran &i, leerían un valor que main sigue modificando.
	 * pthread_create garantiza que todo lo escrito antes de la llamada es
	 * visible para el hilo nuevo (relación happens-before), por eso el hilo
	 * puede leer `identificadores[i]` y `ITERACIONES` sin protección. */
	for (i = 0; i < NUM_FILOSOFOS; i++) {
		identificadores[i] = i;
		rc = pthread_create(&filosofos[i], NULL, filosofo, &identificadores[i]);
		if (rc != 0) {
			fprintf(stderr, "pthread_create(%d): %s\n", i, strerror(rc));
			break;                 /* se unen solo los que sí se crearon */
		}
		creados++;
	}

	/* pthread_join espera a que el hilo termine. Es obligatorio: si main
	 * hiciera return antes, el proceso acabaría y los hilos morirían a medias
	 * (y, además, `identificadores` dejaría de existir en la pila de main). */
	for (i = 0; i < creados; i++) {
		rc = pthread_join(filosofos[i], NULL);
		if (rc != 0) {
			fprintf(stderr, "pthread_join(%d): %s\n", i, strerror(rc));
		}
	}

	printf("variable = %d (esperado %d, incrementos perdidos %d)\n",
	       variable, esperado, esperado - variable);
	if (variable != esperado) {
		printf("=> FALLO: hubo condición de carrera.\n");
	} else {
		printf("=> Esta vez no se perdió ninguno: la carrera es probabilística,\n"
		       "   repita la ejecución o use exclusionMutua01 con un tope grande.\n");
	}

	return 0;
}

/* ------------------------------------------------------------------ *
 * Incremento NO atómico, escrito paso a paso a propósito.
 *
 *  1) LEER   `variable` a un registro del hilo.
 *  2) CEDER   el planificador puede cambiar de hilo justo aquí.
 *  3) ESCRIBIR el valor leído + 1.
 *
 * Si el hilo A lee 3, el hilo B lee 3, y los dos escriben 4, se han hecho dos
 * incrementos y la variable solo ha subido 1.
 *
 * El bucle de sched_yield() solo AGRANDA la ventana entre leer y escribir para
 * que el fallo se vea en (casi) todas las ejecuciones: con solo 25 incrementos
 * el programa dura microsegundos y muchas veces los hilos ni llegan a
 * solaparse. El fallo es real igual sin ceder la CPU; exclusionMutua01 lo
 * demuestra repitiendo millones de veces sin ensanchar nada.
 * ------------------------------------------------------------------ */
static void incrementar_sin_proteccion(void)
{
	int valor_leido = variable;          /* 1) leer */

	for (int k = 0; k < 10; k++) {
		sched_yield();               /* 2) ventana de carrera */
	}

	variable = valor_leido + 1;          /* 3) escribir: pisa lo que hizo otro hilo */
}

void *filosofo(void *arg)
{
	int id = *((int *)arg);

	for (int i = 1; i <= ITERACIONES; i++) {
		incrementar_sin_proteccion();

		/* printf escribe en una salida compartida: la libc ya serializa cada
		 * llamada con su propio cerrojo interno. Por eso las líneas no se
		 * mezclan, pero ESO NO PROTEGE `variable`: el valor que se imprime
		 * puede ser viejo o de otro hilo. Aquí solo se usa para que se vea
		 * la intercalación. */
		printf("Filósofo-%d estoy comiendo : %d. variable = %d\n",
		       id, i, variable);
	}

	return NULL;
}
