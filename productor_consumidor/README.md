Mejoras implementadas en los códigos

Se realizaron modificaciones a los tres códigos originales del problema Productor-Consumidor, manteniendo sus estructuras principales y aplicando mejoras para optimizar su funcionamiento y facilitar su comprensión.

Código 00 – Sincronización entre productores y consumidores

Se incorporó un buffer compartido para almacenar temporalmente las letras generadas por los productores. También se agregaron consumidores, mutex y variables de condición para controlar el acceso al buffer.

Mejoras realizadas:

- Implementación de un buffer con capacidad limitada.
- Incorporación de productores y consumidores.
- Sincronización para evitar que los productores agreguen elementos cuando el buffer está lleno.
- Control para que los consumidores esperen cuando el buffer está vacío.
- Mensajes que permiten identificar cuándo un productor o consumidor debe esperar.

Código 01 – Mejora en la espera de los hilos

Se mantuvo el uso de "std::thread" y se mejoró la forma en que los hilos esperan mediante "condition_variable" con predicados.

Mejoras realizadas:

- Incorporación de productores, consumidores y un buffer compartido.
- Uso de "wait()" con condiciones para controlar cuándo los hilos pueden continuar.
- Control de la cantidad de elementos producidos y consumidos.
- Finalización de los consumidores cuando los productores terminan y el buffer queda vacío.

Código 02 – Control de la cantidad de producción

Se conservó la estructura original basada en la clase "Productor". La principal modificación fue establecer un límite de producción para cada hilo.

Mejoras realizadas:

- Se reemplazó el ciclo infinito por un ciclo "for" limitado a 10 iteraciones.
- Cada productor genera únicamente 10 letras aleatorias.
- Se facilita el control de la cantidad de elementos producidos.
- Los hilos finalizan después de completar su trabajo.

Conclusión:

Las modificaciones permitieron mejorar distintos aspectos de los códigos originales. El código 00 se enfocó en la sincronización, el código 01 en mejorar las condiciones de espera de los hilos y el código 02 en controlar la cantidad de producción. De esta manera, se conservaron las estructuras principales de cada programa y se aplicaron mejoras específicas para cada versión.
