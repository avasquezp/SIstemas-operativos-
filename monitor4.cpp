#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

// g++ monitor04.cpp -o monitor04 -pthread

constexpr int NUM_HILOS = 10;   // [CAMBIADO] #define -> constexpr

// int cont = 0;      // [ELIMINADO] deja de ser global
// mutex flag;        // [ELIMINADO] deja de ser global

class Monitor {
private:
    mutex flag;         // [CAMBIADO] el mutex ahora es atributo privado
    int cont = 0;       // [CAMBIADO] cont pasa a atributo privado

public:
    void inc() {
        lock_guard<mutex> lock(flag);   // [CAMBIADO] lock()/unlock() -> lock_guard
        for (int i = 0; i < 10; i++) {
            cont++;
        }
        cout << cont << endl;   // [SE MANTIENE] ya estaba dentro del lock
    }

    int getCont() {             // [AÑADIDO] lectura sincronizada
        lock_guard<mutex> lock(flag);
        return cont;
    }
};

class Hilo {
private:
    Monitor* monitor;
    thread t;

    void run_thread() {
        monitor->inc();
    }

public:
    Hilo(Monitor* m) {          // [SE MANTIENE] ya recibía el monitor
        monitor = m;
        t = thread(&Hilo::run_thread, this);
    }

    void join_thread() {
        t.join();
    }
};

int main() {
    Monitor* mo = new Monitor;      // [AÑADIDO] LA LÍNEA QUE FALTABA (corrige el bug)
    Hilo* hilos[NUM_HILOS];

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i] = new Hilo(mo);    // [SE MANTIENE] ya pasaba el monitor
    }

    for (int i = 0; i < NUM_HILOS; i++) {
        hilos[i]->join_thread();
    }

    cout << "Valor final de cont = " << mo->getCont() << endl;   // [AÑADIDO]

    for (int i = 0; i < NUM_HILOS; i++) delete hilos[i];   // [AÑADIDO]
    delete mo;                                             // [AÑADIDO]
    return 0;
}
