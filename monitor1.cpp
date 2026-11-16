#include <iostream>
#include <thread>   // [SE MANTIENE] ya existía en monitor01
#include <mutex>    // [AÑADIDO] exclusión mutua

using namespace std;

// g++ monitor01.cpp -o monitor01 -pthread

constexpr int NUM_HILOS = 10;   // [CAMBIADO] #define -> constexpr

// int cont = 0;   // [ELIMINADO] deja de ser global

class Monitor {
private:
    mutex mtx;          // [AÑADIDO] mutex interno
    int cont = 0;       // [CAMBIADO] cont pasa a atributo privado

public:
    void inc() {
        lock_guard<mutex> lock(mtx);   // [AÑADIDO] bloqueo automático al entrar
        for (int i = 0; i < 10; i++) {
            cont++;
        }
        cout << cont << endl;   // [CAMBIADO] la impresión entra en el lock
    }

    int getCont() {                 // [AÑADIDO] lectura sincronizada
        lock_guard<mutex> lock(mtx);
        return cont;
    }
};

class Hilo {
private:
    Monitor* monitor;
    thread t;

    void run_thread() {
        monitor->inc();                 // [CAMBIADO] ya no imprime aquí
    }

public:
    Hilo(Monitor* m) {                  // [CAMBIADO] recibe monitor compartido
        monitor = m;
        t = thread(&Hilo::run_thread, this);
    }

    void join_thread() {
        t.join();
    }
};

int main() {
    Monitor* mo = new Monitor;              // [AÑADIDO] un solo monitor compartido
    Hilo* hilos[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i] = new Hilo(mo);            // [CAMBIADO] se le pasa el monitor
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i]->join_thread();
    }

    cout << "Valor final de cont = " << mo->getCont() << endl;   // [AÑADIDO]

    for (int i = 0; i < NUM_HILOS; i++) delete hilos[i];   // [AÑADIDO] libera memoria
    delete mo;                                             // [AÑADIDO]
    return 0;
}
