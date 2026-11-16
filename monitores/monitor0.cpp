#include <iostream>
#include <thread>   // [AÑADIDO] hilos reales
#include <mutex>    // [AÑADIDO] exclusión mutua
using namespace std;

// g++ monitor00.cpp -o monitor00 -pthread   // [CAMBIADO] -lpthread por -pthread

constexpr int NUM_HILOS = 10;   // [CAMBIADO] #define -> constexpr

// int cont = 0;   // [ELIMINADO] ya no es global

class Monitor {
    private:
        mutex mtx;          // [AÑADIDO] mutex interno del monitor
        int cont = 0;       // [CAMBIADO] cont pasa a ser atributo privado

    public:
        void inc() {
            lock_guard<mutex> lock(mtx);   // [AÑADIDO] bloqueo automático al entrar
            for (int i = 0; i < 10; i++) {
                cont++;
            }
        }   // [AÑADIDO] el lock se libera solo al salir

        int getCont() {                     // [AÑADIDO] lectura sincronizada
            lock_guard<mutex> lock(mtx);
            return cont;
        }
};

class Hilo {
    private:
        Monitor* monitor;
        thread t;                           // [AÑADIDO] cada objeto tiene su hilo

        void run_thread() {
            monitor->inc();                 // [CAMBIADO] antes se llamaba run() desde main
            cout << monitor->getCont() << endl;   // [CAMBIADO] imprimir desde el hilo
        }

    public:
        Hilo(Monitor* m) {                  // [CAMBIADO] recibe el monitor compartido
            monitor = m;
            t = thread(&Hilo::run_thread, this);   // [AÑADIDO] lanza el hilo
        }

        void join_thread() {                // [AÑADIDO] espera al hilo
            t.join();
        }
};

int main() {
    Monitor monitor;                        // [AÑADIDO] un único monitor compartido
    Hilo* hilos[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i] = new Hilo(&monitor);      // [CAMBIADO] se le pasa el monitor
    }

    // for (i = 0; i < NUM_HILOS; i++) { hilos[i]->run(); }   // [ELIMINADO] ya no es secuencial

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i]->join_thread();            // [AÑADIDO] espera a todos los hilos
    }

    cout << "Valor final de cont = " << monitor.getCont() << endl;   // [AÑADIDO]

    for (int i = 0; i < NUM_HILOS; i++) delete hilos[i];   // [AÑADIDO] libera memoria
    return 0;
}

