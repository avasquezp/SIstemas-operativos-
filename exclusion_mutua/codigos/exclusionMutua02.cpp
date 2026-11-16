/*
 * exclusionMutua02.cpp  --  EXCLUSIÓN MUTUA CON pthread_mutex_t
 *
 * Qué hace:  Lo mismo que exclusionMutua01 (5 hilos incrementan `tope` veces una
 *            variable global), pero ahora cada acceso a la variable está dentro
 *            de un mutex. El resultado siempre es exactamente 5 * tope.
 *
 * Compilar:  g++ exclusionMutua02.cpp -pthread -Wall -Wextra -o exclusionMutua02
 * Ejecutar:  ./exclusionMutua02      (mismo tope que en el 01, para comparar)
 *
 * Idea clave: un mutex es un cerrojo de exclusión mutua (mutual exclusion).
 *   - pthread_mutex_lock()   : si el cerrojo está libre lo toma; si ya lo tiene
 *                              otro hilo, BLOQUEA al que llama hasta que se libere.
 *   - pthread_mutex_unlock() : libera el cerrojo y despierta a uno de los hilos
 *                              que estaban esperando.
 * El código entre lock y unlock es la SECCIÓN CRÍTICA: solo un hilo puede estar
 * dentro a la vez, así que los tres pasos (leer, sumar, escribir) se ejecutan
 * sin que nadie se cuele en medio. Eso es lo que convierte la operación en
 * atómica *respecto de los demás hilos*.
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   /* isatty: distinguir entrada interactiva de una tubería */

#define NUM_FILOSOFOS 5
#define TOPE_POR_DEFECTO 10
#define TOPE_MAXIMO 1000000

void *filosofo(void *arg);

int variable = 0;
int tope = TOPE_POR_DEFECTO;

/* ------------------------------------------------------------------ *
 * EL MUTEX. Se declara global porque lo comparten todos los hilos.
 *
 * Forma 1 (la de este programa): inicialización dinámica con
 *   pthread_mutex_init(&flag, NULL)  ->  y destroy al final.
 *   El segundo argumento son los atributos: NULL = mutex por defecto.
 *
 * Forma 2 (equivalente y más simple): inicialización estática con
 *   pthread_mutex_t flag = PTHREAD_MUTEX_INITIALIZER;
 *   En ese caso no hace falta ni init ni destroy.
 *
 * Con el mutex por defecto (normal) el estándar no comprueba errores de uso:
 * si el mismo hilo hace lock dos veces sin unlock, se queda bloqueado para
 * siempre (autobloqueo) sin avisar. PTHREAD_MUTEX_ERRORCHECK y
 * PTHREAD_MUTEX_RECURSIVE existen para depurar / permitir relock.
 * ------------------------------------------------------------------ */
pthread_mutex_t flag;

/* Envuelve las llamadas POSIX, que devuelven 0 o un código de error (NO usan
 * errno), para no ignorarlas silenciosamente como hacía el assert(!result):
 * assert desaparece al compilar con -DNDEBUG y entonces un fallo pasa inadvertido. */
static void comprobar(int rc, const char *operacion)
{
	if (rc != 0) {
		fprintf(stderr, "%s: %s\n", operacion, strerror(rc));
		exit(EXIT_FAILURE);
	}
}

static int leer_tope(void)
{
	int leido;

	printf("Ingrese tope : ");
	fflush(stdout);
	if (scanf("%d", &leido) != 1) {
		fprintf(stderr, "Entrada inválida; se usa tope = %d\n", TOPE_POR_DEFECTO);
		return TOPE_POR_DEFECTO;
	}
	/* Si la entrada no es un terminal (por ejemplo `echo 200000 | ./em02`), no
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
	long esperado, obtenido;

	tope = leer_tope();

	printf("variable = %d\n", variable);
	esperado = (long)NUM_FILOSOFOS * (long)tope;
	printf("tope = %d, valor esperado = %ld\n", tope, esperado);

	/* Se inicializa ANTES de crear los hilos: si un hilo hiciera lock sobre un
	 * mutex sin inicializar el comportamiento es indefinido. */
	comprobar(pthread_mutex_init(&flag, NULL), "pthread_mutex_init");

	for (i = 0; i < NUM_FILOSOFOS; i++) {
		identificadores[i] = i;
		rc = pthread_create(&filosofos[i], NULL, filosofo, &identificadores[i]);
		comprobar(rc, "pthread_create");
		creados++;
	}

	for (i = 0; i < creados; i++) {
		comprobar(pthread_join(filosofos[i], NULL), "pthread_join");
	}

	/* Se destruye DESPUÉS del último join: destruir un mutex que un hilo puede
	 * estar usando (o tener tomado) es un error. */
	comprobar(pthread_mutex_destroy(&flag), "pthread_mutex_destroy");

	obtenido = (long)variable;
	printf("variable = %ld (esperado %ld, incrementos perdidos %ld)\n",
	       obtenido, esperado, esperado - obtenido);

	/* Con exclusión mutua correcta esto SIEMPRE da 0 pérdidas. Si no, es que
	 * falta proteger algún acceso (por ejemplo, un lock en el hilo y otro
	 * acceso a `variable` fuera de la sección crítica). */
	if (obtenido == esperado) {
		printf("=> CORRECTO: la exclusión mutua funcionó.\n");
	} else {
		printf("=> FALLO: todavía hay un acceso sin proteger.\n");
		return EXIT_FAILURE;
	}

	return 0;
}

void *filosofo(void *arg)
{
	int id = *((int *)arg);   /* la dirección es propia de este hilo */

	for (int i = 1; i <= tope; i++) {

		comprobar(pthread_mutex_lock(&flag), "pthread_mutex_lock");

		/* ============ INICIO DE LA SECCIÓN CRÍTICA ============
		 * Los TRES pasos (leer, sumar, escribir) están dentro del cerrojo.
		 * Proteger solo la escritura NO serviría de nada: el otro hilo podría
		 * leer el valor viejo entre nuestra lectura y nuestra escritura.
		 *
		 * Regla: toda lectura o escritura del dato compartido debe estar
		 * dentro de la misma sección crítica. Aquí no se imprime nada: printf
		 * bloquea y haría la sección crítica mucho más larga, serializando a
		 * los 5 hilos también en la E/S, no solo en el incremento. */
		variable = variable + 1;
		/* ============ FIN DE LA SECCIÓN CRÍTICA ============ */

		/* El unlock va SIEMPRE, también si hubiera un return o un error dentro
		 * de la sección crítica. Si se sale sin liberar, el resto de hilos se
		 * queda esperando para siempre: interbloqueo (deadlock).
		 * Para código con salidas anticipadas, C++ ofrece RAII
		 * (std::lock_guard<std::mutex>) y POSIX ofrece pthread_cleanup_push()
		 * para liberar si el hilo es cancelado. */
		comprobar(pthread_mutex_unlock(&flag), "pthread_mutex_unlock");
	}

	(void)id;   /* sin printf no se usa; se conserva la firma de pthread_create */
	return NULL;
}
