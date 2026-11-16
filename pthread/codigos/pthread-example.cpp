/*
 * pthread-example.cpp  --  Ejemplo mínimo de pthreads
 *
 * Qué hace:  main crea 5 hilos. Cada hilo imprime su índice, duerme un número
 *            aleatorio de segundos (1..5) e imprime que terminó. main espera a
 *            que terminen todos con pthread_join y después termina.
 *            El orden de los mensajes cambia en cada ejecución: los hilos se
 *            ejecutan en paralelo y el planificador decide cuándo corre cada uno.
 *
 * Compilar:  g++ pthread-example.cpp -pthread -Wall -Wextra -o pthread-example
 * Ejecutar:  ./pthread-example
 */

#include <stdio.h>    /* printf, perror, fprintf, stdout */
#include <stdlib.h>   /* EXIT_SUCCESS / EXIT_FAILURE, rand_r */
#include <pthread.h>  /* pthread_create, pthread_join, pthread_self */
#include <unistd.h>   /* sleep */
#include <time.h>     /* time: semilla distinta en cada ejecución */
#include <errno.h>    /* errno: perror lo usa */

#define NUM_THREADS 5

/* Semilla base compartida. Se escribe UNA vez en main antes de crear los hilos
 * y ya no se modifica: así los hilos pueden leerla sin cerrojo, porque
 * pthread_create establece la relación happens-before (todo lo escrito antes
 * de crear el hilo es visible para él). */
static unsigned int semilla_base;

/* Función que ejecuta cada hilo.
 * Su firma la impone pthread_create: recibe un void* y devuelve un void*.
 * Aquí el void* apunta a un int con el índice del hilo. */
void *perform_work(void *arguments)
{
	int index = *((int *)arguments);

	/* Cada hilo usa su PROPIO estado de aleatoriedad en la pila. rand() usa un
	 * único estado global y no es seguro entre hilos (dos hilos podrían
	 * obtener el mismo "aleatorio"); rand_r() recibe la semilla por puntero y
	 * no comparte nada, así que es la opción correcta en código concurrente. */
	unsigned int semilla = semilla_base + (unsigned int)index * 7919u;
	int sleep_time = 1 + (int)(rand_r(&semilla) % NUM_THREADS);

	printf("THREAD %d (tid %lu): Started.\n", index,
	       (unsigned long)pthread_self());
	printf("THREAD %d: Will be sleeping for %d seconds.\n", index, sleep_time);

	/* sleep() suspende SOLO este hilo; mientras tanto main y el resto siguen
	 * avanzando. Es un punto de cancelación: si otro hilo lo cancelara, el
	 * hilo podría morir aquí. */
	sleep((unsigned int)sleep_time);

	printf("THREAD %d: Ended.\n", index);

	/* El valor devuelto llega a main como segundo argumento de pthread_join.
	 * Aquí no hace falta devolver nada. */
	return NULL;
}

int main(void)
{
	pthread_t threads[NUM_THREADS];
	int thread_args[NUM_THREADS];   /* un entero por hilo */
	int i;
	int result_code;

	/* Sin srand(), rand() empieza siempre con la misma semilla y el programa
	 * da siempre los mismos tiempos. Con rand_r() la semilla se pasa a mano;
	 * se deriva de la hora para que cambie en cada ejecución. */
	semilla_base = (unsigned int)time(NULL);

	/* Crear los hilos uno a uno. Al volver de pthread_create el hilo YA puede
	 * estar ejecutándose (incluso puede haber terminado antes de que main
	 * cree el siguiente). */
	for (i = 0; i < NUM_THREADS; i++) {
		printf("IN MAIN: Creating thread %d.\n", i);
		thread_args[i] = i;   /* hueco propio: si se pasara &i, todos los
		                       * hilos leerían el i cambiante de main */
		result_code = pthread_create(&threads[i], NULL, perform_work,
		                             &thread_args[i]);
		/* Las funciones pthread NO usan errno: devuelven el código de error.
		 * Por eso se asigna a errno antes de perror() (o se usa strerror). */
		if (result_code != 0) {
			errno = result_code;
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	printf("IN MAIN: All threads are created.\n");

	/* Esperar a cada hilo. join hace dos cosas: bloquea hasta que el hilo
	 * termina y libera sus recursos (sin join, el hilo queda "zombi" y pierde
	 * su pila). Es obligatorio esperar: si main hiciera return antes, el
	 * proceso terminaría y mataría a los hilos a mitad de su sleep(). */
	for (i = 0; i < NUM_THREADS; i++) {
		result_code = pthread_join(threads[i], NULL);   /* NULL = no me interesa el return */
		if (result_code != 0) {
			errno = result_code;
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}
		printf("IN MAIN: Thread %d has ended.\n", i);
	}

	printf("MAIN program has ended.\n");
	return EXIT_SUCCESS;
}
