
#include<stdio.h>
#include<thread>
#include<stdlib.h>
#include<mutex>
#include<condition_variable>
#include<ctime>

using namespace std;

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

mutex mutexBuffer;
condition_variable hayDatos;
condition_variable hayEspacio;

void productor(int k);
void consumidor(int k);

int main(void)
{
    thread productores[NUM_PRODUCTORES];
    thread consumidores[NUM_CONSUMIDORES];

    srand(time(NULL));

    for (int i=0; i<NUM_CONSUMIDORES; i++) {
        consumidores[i] = thread(consumidor, i);
    }

    for (int i=0; i<NUM_PRODUCTORES; i++) {
        productores[i] = thread(productor, i);
    }

    for (int i=0; i<NUM_PRODUCTORES; i++) {
        productores[i].join();
    }

    for (int i=0; i<NUM_CONSUMIDORES; i++) {
        consumidores[i].join();
    }

    printf("\n--- RESULTADOS ---\n");
    printf("Total producido: %d\n", producidos);
    printf("Total consumido: %d\n", consumidos);
    printf("Elementos restantes: %d\n", cantidad);

    return 0;
}

void productor(int k)
{
    int aleatorio;

    for (int i=1; i<=CANTIDAD; i++) {

        unique_lock<mutex> lock(mutexBuffer);

        hayEspacio.wait(lock, [] { return cantidad < TAM_BUFFER; });

        aleatorio = rand() % 26;

        buffer[entrada] = letras[aleatorio];
        entrada = (entrada + 1) % TAM_BUFFER;
        cantidad++;
        producidos++;

        printf("Productor-%d produjo: %c\n", k, letras[aleatorio]);
        
        hayDatos.notify_one();
    }

    {
        lock_guard<mutex> lock(mutexBuffer);
        productoresTerminados++;
    }

    hayDatos.notify_all();
}

void consumidor(int k)
{
    char letra;
    int numero;

    while (true) {

        unique_lock<mutex> lock(mutexBuffer);

        hayDatos.wait(lock, [] {
            return cantidad > 0 || productoresTerminados == NUM_PRODUCTORES;
        });

        if (cantidad == 0 && productoresTerminados == NUM_PRODUCTORES) {
            break;
        }

        letra = buffer[salida];
        salida = (salida + 1) % TAM_BUFFER;
        cantidad--;

        consumidos++;
        numero = consumidos;

        printf("Consumidor-%d consumio: %c | Consumo numero: %d\n",
               k, letra, numero);

        hayEspacio.notify_one();
    }
}
