#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

// g++ monitor03.cpp -o monitor03 -pthread

constexpr int NUM_HILOS = 10;   // [CAMBIADO] #define -> constexpr

// int cont = 0;      // [ELIMINADO] deja de ser global
// mutex flag;        // [ELIMINADO] deja de ser global

class Monitor {
private:
    mutex flag;         // [CAMBIADO] el mutex ahora es atributo privado
    int cont = 0;       // [CAMBIADO] cont pasa a atributo privado

public:
    void inc() {
        lock_guard<mutex> lock(flag);   // [AÑADIDO] el lock vuelve aquí (estaba fuera)
        for (int i = 0; i < 10; i++) {
            cont++;
        }
        cout << cont << endl;   // [CAMBIADO] la impresión también queda protegida
    }
};

class Hilo {
private:
    Monitor* monitor;
    thread t;

    void run_thread() {
        // flag.lock();     // [ELIMINADO] ya no se bloquea desde el hilo
        monitor->inc();
        // cout<<cont<<endl;   // [ELIMINADO] se movió dentro del monitor
        // flag.unlock();   // [ELIMINADO]
    }

public:
    Hilo(Monitor* m) {          // [CAMBIADO] recibe monitor compartido
        monitor = m;
        t = thread(&Hilo::run_thread, this);
    }

    void join_thread() {
        t.join();
    }
};

int main() {
    Monitor* mo = new Monitor;      // [AÑADIDO] un solo monitor compartido
    Hilo* hilos[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i] = new Hilo(mo);    // [CAMBIADO] se le pasa el monitor
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i]->join_thread();
    }

    for (int i = 0; i < NUM_HILOS; i++) delete hilos[i];   // [AÑADIDO]
    delete mo;                                             // [AÑADIDO]
    return 0;
}
