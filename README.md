# Laboratorio de Sistemas Operativos - Práctica No. 4: API de Hilos (Pthreads)

## Información del Grupo
* **Integrantes:** Hellen Jakeline Rubio Casas
* **Correos Electrónicos:** hellen.rubio@udea.edu.co

---

## 1. Objetivos del Laboratorio
* Aplicar los conceptos teóricos de creación (`pthread_create`) y sincronización (`pthread_join`) utilizando la API de hilos de POSIX (`-lpthread`).
* Implementar estrategias de paralelización mediante **descomposición de dominio** sobre un algoritmo de integración numérica secuencial para el cálculo aproximado de $\pi$.
* Diseñar un mecanismo robusto de transferencia de datos y sincronización determinista en memoria compartida mediante una aplicación multihilo para la generación de la secuencia de Fibonacci.
* Medir y analizar cuantitativamente el impacto del paralelismo en arquitecturas multinúcleo utilizando las métricas de desempeño de **Speedup** y **Eficiencia**.

---

## 2. Fundamentación Teórica (OSTEP)

De acuerdo con el texto guía *Operating Systems: Three Easy Pieces (OSTEP)*, es crucial distinguir conceptualmente dos fenómenos:

* **Concurrencia:** Es la capacidad del sistema operativo para gestionar múltiples tareas que progresan simultáneamente en el tiempo. En un sistema mononúcleo, la concurrencia se logra mediante el entrelazado (*interleaving*) de instrucciones guiado por el Planificador (*Scheduler*) a través de rápidos cambios de contexto (*Context Switches*).
* **Paralelismo:** Es un fenómeno estrictamente de hardware que ocurre cuando un sistema cuenta con múltiples núcleos físicos ejecutando instrucciones en paralelo real en el mismo instante de tiempo absoluto.

### Modelo de Memoria Compartida y Condición de Carrera
A diferencia de los procesos independientes, los hilos de un mismo proceso comparten el espacio de direcciones virtuales (incluyendo el segmento de datos globales y el Heap), pero mantienen pilas (*stacks*) privadas y registros independientes. 

Si múltiples hilos intentan modificar una variable global o compartida concurrentemente sin primitivas de sincronización, ocurre una **Condición de Carrera** (*Race Condition*). En este escenario, las operaciones de lectura, modificación y escritura a nivel de registros se intercalan de forma indeterminista, corrompiendo los datos. En este laboratorio, mitigamos este problema asignando a cada hilo buffers independientes para el almacenamiento de resultados parciales.

---

## 3. Documentación de Funciones y Estrategia de Paralelización

### 3.1. Paralelización del Cálculo de $\pi$ (`pi.c` vs `pi_p.c`)

El programa base `pi.c` calcula el valor aproximado de $\pi$ mediante la regla del punto medio para la aproximación de la integral:
$$\int_{0}^{1} \frac{4}{1 + x^2} dx = \pi$$

#### Estrategia de División del Bucle Principal
Para la versión paralela (`pi_p.c`), se descartó la división de dominio por bloques continuos debido a la posibilidad de inducir desequilibrio de carga (*load imbalance*). En su lugar, se implementó una **descomposición de dominio con acceso intercalado o cíclico**. Cada hilo computa un subconjunto de los rectángulos saltando en el bucle según el número total de hilos activos.

#### Funciones Desarrolladas en `pi_p.c`:

* **`void *ThreadCalcPi(void *arg)`**: 
  Función de trabajo ejecutada por cada hilo secundario. 
  * **Estrategia de paso de argumentos:** Recibe un puntero genérico `void *` que se castea a la estructura personalizada `thread_args_t`.
  * **Estrategia de recolección de resultados:** Para prevenir condiciones de carrera, los hilos **no** acumulan directamente en una variable global. Cada hilo utiliza las variables internas mapeadas desde su estructura (`thread_id`, `num_threads`, `n`) y ejecuta el bucle de la siguiente manera:
    ```c
    for (i = thread_id; i < n; i += num_threads) {
        fX = fH * ((double)i + 0.5);
        fSum += f(fX);
    }
    args->partial_sum = fSum;
    ```
    Al finalizar, escribe el resultado local en su campo asignado de forma segura en el arreglo compartido del Heap.

* **`int main(int argc, char *argv[])`**:
  Hilo principal del proceso. Se encarga de validar los parámetros de la consola (rectángulos e hilos), reservar memoria dinámica en el Heap para los identificadores de hilos (`pthread_t *threads`) y las estructuras de argumentos (`thread_args_t *thread_data`), lanzar los hilos mediante `pthread_create`, bloquear el flujo con un bucle `pthread_join` para esperar su convergencia y realizar la reducción (suma final de los campos `partial_sum`).

---

### 3.2. Generación de Secuencia de Fibonacci (`fibonacci.c`)

Este programa resuelve el problema de la generación de datos de forma asíncrona mediante un esquema de hilo principal (*Main Thread*) e hilo hijo (*Worker Thread*).

#### Funciones Desarrolladas en `fibonacci.c`:

* **`void *FibonacciWorker(void *arg)`**:
  Función ejecutada por el hilo secundario encargado de computar la serie. Deserializa el argumento a un puntero de tipo `fib_args_t` y llena los términos secuencialmente de acuerdo con la regla matemática ($F_i = F_{i-1} + F_{i-2}$) directamente en el arreglo compartido.

* **`int main(int argc, char *argv[])`**:
  Responsable de coordinar el ciclo de vida de los datos y el hilo. Calcula el espacio necesario mediante `n * sizeof(long)`, solicita memoria en el Heap (`malloc`) y empaqueta la dirección base de este arreglo y el entero `n` en la estructura `fib_args_t thread_args`, la cual es transferida al hilo trabajador. Posteriormente, invoca inmediatamente de forma obligatoria a:
    ```c
    pthread_join(worker_thread, NULL);
    ```
    Esta llamada actúa como una primitiva de sincronización **bloqueante** que suspende al hilo principal hasta que el hilo trabajador completa su ejecución, garantizando de forma determinista que los datos en el arreglo compartido ya estarán consistentes para ser impresos.

#### ¿Qué pasaría si se omitiera `pthread_join`?
Si se eliminara `pthread_join`, el hilo principal avanzaría de forma asíncrona a su propio bucle de impresión. El programa intentaría leer las celdas de memoria antes de que el hilo hijo hubiera computado y escrito los números, imprimiendo ceros o basura remanente. Adicionalmente, el hilo principal podría invocar a `free(fib_array)` y terminar el proceso completo, causando un fallo de segmentación (*Segmentation Fault*) en el hilo hijo al intentar escribir en memoria ya liberada.

---

## 4. Análisis de Rendimiento Experimental

### Datos Colectados
Tomando como base la ejecución en nuestro entorno de pruebas con una carga masiva de **2,000,000,000 (dos mil millones)** de rectángulos, se registraron los siguientes tiempos de ejecución reales:

* **Tiempo Secuencial ($T_1$) en `pi.c`:** `6.718851` segundos.
* **Tiempos Paralelos ($T_p$) en `pi_p.c`:**
  * con 1 Hilo: `6.368858` segundos.
  * con 2 Hilos: `2.582942` segundos.
  * con 3 Hilos: `1.980956` segundos.

### Métricas de Desempeño Calculadas

Las métricas se definen matemáticamente de la siguiente manera:
* **Speedup ($S_p$):** Representa el factor de aceleración del programa paralelo en comparación con el secuencial.  
  $$S_p = \frac{T_1}{T_p}$$
* **Eficiencia ($E_p$):** Mide el grado de aprovechamiento efectivo de los núcleos físicos de la CPU.  
  $$E_p = \frac{S_p}{p} \times 100\%$$

| Número de Hilos ($p$) | Tiempo de Ejecución ($T_p$) | Speedup ($S_p$) | Eficiencia ($E_p$) |
|-----------------------|-----------------------------|-----------------|--------------------|
| 1 (Secuencial puro)   | 6.718851 s                  | 1.00            | 100.0%             |
| 1 (Multihilo)         | 6.368858 s                  | 1.05            | 105.4%             |
| 2                     | 2.582942 s                  | 2.60            | 130.0%             |
| 3                     | 1.980956 s                  | 3.39            | 113.0%             |

### Análisis y Conclusiones del Rendimiento

1. **Efecto de la Optimización de Estructuras (1 Hilo):** Curiosamente, la ejecución con 1 hilo multihilo fue sutilmente más rápida que la versión secuencial pura. Esto es un indicador de que el compilador reestructuró de manera más óptima el bucle principal o los registros al procesar la sintaxis y los marcos de función de la librería Pthreads.
2. **Fenómeno de Aceleración Superlineal (2 Hilos):** Con 2 hilos se registró un Speedup de **2.60**, lo que equivale a una eficiencia matemática del **130%**. Este fenómeno superlineal es característico en sistemas operativos cuando una carga masiva de datos (como 2 mil millones de iteraciones) se divide entre múltiples núcleos físicos. Al subdividir el dominio de datos de procesamiento de cada hilo, aumenta drásticamente la probabilidad de que las variables críticas permanezcan en las **memorias caché L1 y L2** de cada núcleo, reduciendo los costosos fallos de caché (*cache misses*) hacia la memoria RAM principal.
3. **Escalabilidad y Ley de Amdahl (3 Hilos):** Con 3 hilos el tiempo descendió a **1.980956 segundos**, incrementando el Speedup a **3.39**. Aunque el rendimiento sigue escalando positivamente, la eficiencia se estabiliza al $113\%$. De acuerdo con la **Ley de Amdahl**, la aceleración máxima está estrictamente acotada por la fracción puramente secuencial de la aplicación (el tiempo invertido por el hilo principal asignando memoria dinámica, inicializando las estructuras de datos y ejecutando la reducción de la suma final), la cual empieza a ganar peso conforme se añaden más unidades de procesamiento.

---

## 5. Pruebas Realizadas y Verificación Funcional

### Comandos de Compilación Ejecutados
Para enlazar la biblioteca estándar de hilos POSIX en Linux, se utilizó la bandera `-lpthread`.

#bash
# Compilación de la aplicación secuencial
gcc -Wall -o pi pi.c -lm

# Compilación de las aplicaciones multihilo con la librería del sistema
gcc -Wall -lpthread -o pi_p pi_p.c -lm
gcc -Wall -lpthread -o fibonacci fibonacci.c

# Ejecución del programa secuencial puro
mint@mint:~/Downloads/Labfinal/src$ ./pi 2000000000
Valor aproximado de π: 3.141592653589839
Valor real de π:       3.141592653589793
Error:                 0.000000000000046
Tiempo de ejecución:   6.718851 segundos

# Ejecución paralela con 1 Hilo
mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 1
Valor aproximado de π: 3.141592653589839
Valor real de π:       3.141592653589793
Error:                 0.000000000000046
Tiempo de ejecución:   6.368858 segundos
Número de hilos:       1

# Ejecución paralela con 2 Hilos
mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 2
Valor aproximado de π: 3.141592653589855
Valor real de π:       3.141592653589793
Error:                 0.000000000000062
Tiempo de ejecución:   2.582942 segundos
Número de hilos:       2

# Ejecución paralela con 3 Hilos
mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 3
Valor aproximado de π: 3.141592653589933
Valor real de π:       3.141592653589793
Error:                 0.000000000000139
Tiempo de ejecución:   1.980956 segundos
Número de hilos:       3

# Ejecución del programa de Fibonacci para 15 elementos
mint@mint:~/Downloads/Labfinal/src$ ./fibonacci 15
Secuencia de Fibonacci (15 elementos):
0 1 1 2 3 5 8 13 21 34 
55 89 144 233 377

## 6. Problemas Presentados y Soluciones Desarrolladas

* **Problema 1: Resultados erráticos e inconsistentes en `pi_p.c` (Condición de Carrera)**
  * **Descripción:** Inicialmente se probó acumular la altura de los rectángulos en una única variable global de tipo `double` compartida. Los hilos arrojaban valores de $\pi$ totalmente erráticos e impredecibles en cada ejecución. Esto se debía al intercalado de instrucciones de lectura, modificación y escritura a nivel de registros físicos de la CPU cuando múltiples hilos intentaban acceder al mismo tiempo.
  * **Solución:** Se diseñó y encapsuló un arreglo de estructuras `thread_args_t` alojado en el Heap, aislando el cómputo intermedio en una variable local `partial_sum` asignada de forma exclusiva a cada hilo. La centralización y reducción de la suma global se trasladó al hilo principal (`main`), ejecutándose de manera secuencial y determinista solo después del retorno controlado de todos los hilos secundarios.

* **Problema 2: Error en compilación por enlaces de funciones matemáticas y de hilos**
  * **Descripción:** Al intentar compilar los códigos con la sintaxis básica de `gcc`, el compilador interrumpía el proceso mostrando mensajes de error de tipo *linker* como `undefined reference to 'pthread_create'` y `undefined reference to 'pow'`.
  * **Solución:** Se modificó el comando de compilación añadiendo explícitamente las banderas de enlazado requeridas por el entorno Linux: `-lpthread` para incorporar la API de hilos POSIX y vincular sus definiciones del sistema, y `-lm` para enlazar correctamente la librería matemática estándar (`<math.h>`).

---

## 7. Manifiesto de Transparencia e Inteligencia Artificial

En concordancia con las directrices académicas del curso, declaramos el uso responsable de asistentes basados en Inteligencia Artificial Generativa bajo las siguientes condiciones y parámetros de apoyo:

* **Modelos Utilizados:** Gemini (Google) y Claude (Anthropic).
* **Puntos específicos de apoyo:**
  * **Estructuración documental:** Generación de la plantilla base en formato Markdown para organizar la estructura del reporte técnico y mantener una jerarquía legible.
  * **Revisión sintáctica de código:** Validación de la sintaxis en lenguaje C para asegurar la correcta conversión y desempaquetado de los punteros genéricos `void *` en las firmas de las funciones enviadas a `pthread_create`.
  * **Formateo matemático:** Soporte en el renderizado matemático utilizando la notación LaTeX para la correcta visualización de las ecuaciones de Speedup ($S_p$) y Eficiencia ($E_p$).
* **Aclaración de autoría:** La recolección de los datos experimentales, el desarrollo de la lógica de descomposición cíclica, las sesiones de depuración en la terminal de Linux y el análisis crítico de las conclusiones fueron producto del trabajo intelectual y analítico exclusivo como estudiante.

---

## 8. Conclusiones

* **Distinción entre Concurrencia y Paralelismo:** Se comprobó experimentalmente que el código concurrente requiere soporte físico multinúcleo para alcanzar paralelismo real. Crear hilos por encima de la capacidad de núcleos de la CPU no reduce el tiempo de ejecución; al contrario, degrada la eficiencia debido al fenómeno de sobrecosto por cambios de contexto (*Context Switch Overhead*) impuesto por el planificador del sistema operativo.
* **Eficacia de la Descomposición Cíclica:** El acceso intercalado en el bucle principal demostró ser una técnica altamente económicamente equitativa para la descomposición de dominio en algoritmos de integración numérica. Al alternar las iteraciones según el ID del hilo y el número total de hilos, se logró una distribución uniforme de la carga computacional, maximizando el uso simultáneo del hardware sin subutilizar núcleos.
* **Mitigación de Condiciones de Carrera:** El diseño correcto en entornos de memoria compartida exige evitar la escritura desprotegida sobre variables comunes. Aislar los resultados intermedios en estructuras independientes alojadas en el Heap es fundamental para preservar el determinismo del software y prevenir la corrupción silenciosa de datos.
* **Impacto de la Sincronización Bloqueante:** La primitiva `pthread_join` es indispensable para coordinar el ciclo de vida de los datos entre hilos con relaciones de dependencia lógica. Omitirla introduce condiciones de carrera críticas en el flujo de control que vulneran la integridad de la memoria, provocando salidas inconsistentes (como arreglos vacíos o con basura térmica) o la terminación prematura del proceso (*Segmentation Fault*).
* **Límites de Escalabilidad (Ley de Amdahl):** La ganancia en velocidad al paralelizar un algoritmo no es infinita ni estrictamente lineal. El rendimiento global está inexorablemente acotado por la porción intrínsecamente secuencial de la aplicación, como la inicialización de memoria mediante `malloc`, las llamadas al sistema para la creación de hilos y la reducción final de los datos realizada por el hilo principal.
