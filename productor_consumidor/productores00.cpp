#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<time.h>

#define NUM_PRODUCTORES 2
#define NUM_CONSUMIDORES 2
#define TAM_BUFFER 5
#define CANTIDAD 5

const char letras[26]={'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};

char buffer[TAM_BUFFER];

int entrada = 0;
int salida = 0;
int cantidad = 0;
int productoresTerminados = 0;

int producidos = 0;
int consumidos = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t hayDatos = PTHREAD_COND_INITIALIZER;
pthread_cond_t hayEspacio = PTHREAD_COND_INITIALIZER;

void *productor(void *arg);
void *consumidor(void *arg);

int main(void)
{
    pthread_t productores[NUM_PRODUCTORES];
    pthread_t consumidores[NUM_CONSUMIDORES];

    int idProductores[NUM_PRODUCTORES];
    int idConsumidores[NUM_CONSUMIDORES];

    int i;

    srand(time(NULL));

    for (i=0; i<NUM_CONSUMIDORES; i++) {
        idConsumidores[i] = i;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    for (i=0; i<NUM_PRODUCTORES; i++) {
        idProductores[i] = i;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    for (i=0; i<NUM_PRODUCTORES; i++) {
        pthread_join(productores[i], NULL);
    }

    for (i=0; i<NUM_CONSUMIDORES; i++) {
        pthread_join(consumidores[i], NULL);
    }

    printf("\n--- RESULTADOS ---\n");
    printf("Total producido: %d\n", producidos);
    printf("Total consumido: %d\n", consumidos);
    printf("Elementos restantes: %d\n", cantidad);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&hayDatos);
    pthread_cond_destroy(&hayEspacio);

    return 0;
}

void *productor(void *arg)
{
    int arg2 = *((int *)arg);
    int aleatorio;

    for (int i=1; i<=CANTIDAD; i++) {

        pthread_mutex_lock(&mutex);

        while (cantidad == TAM_BUFFER) {
            printf("Productor-%d esperando: buffer lleno\n", arg2);
            pthread_cond_wait(&hayEspacio, &mutex);
        }

        aleatorio = rand() % 26;

        buffer[entrada] = letras[aleatorio];
        entrada = (entrada + 1) % TAM_BUFFER;
        cantidad++;

        producidos++;

        printf("Productor-%d produjo: %c\n", arg2, letras[aleatorio]);

        pthread_cond_signal(&hayDatos);

        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    productoresTerminados++;
    pthread_cond_broadcast(&hayDatos);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *consumidor(void *arg)
{
    int arg2 = *((int *)arg);
    char letra;

    while (true) {

        pthread_mutex_lock(&mutex);

        while (cantidad == 0 && productoresTerminados < NUM_PRODUCTORES) {
            printf("Consumidor-%d esperando: buffer vacio\n", arg2);
            pthread_cond_wait(&hayDatos, &mutex);
        }

        if (cantidad == 0 && productoresTerminados == NUM_PRODUCTORES) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        letra = buffer[salida];
        salida = (salida + 1) % TAM_BUFFER;
        cantidad--;

        consumidos++;

        printf("Consumidor-%d consumio: %c\n", arg2, letra);

        pthread_cond_signal(&hayEspacio);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
