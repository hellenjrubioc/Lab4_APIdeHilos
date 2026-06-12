# Laboratorio de Sistemas Operativos - Práctica No. 4: API de Hilos (Pthreads)

## Información del Grupo
* **Integrantes:** Hellen Jakeline Rubio Casas
* **Correos Electrónicos:** hellen.rubio@udea.edu.co
* Se elige omitir ID en repositorio*

---
--

## 1. Objetivos del Laboratorio
* Aplicar los conceptos teóricos de creación (`pthread_create`) y sincronización (`pthread_join`) utilizando la API de hilos de POSIX (`-lpthread`).
* Implementar estrategias de paralelización mediante **descomposición de dominio** sobre un algoritmo de integración numérica secuencial para el cálculo aproximado de $\pi$.
* Diseñar un mecanismo robusto de transferencia de datos y sincronización determinista en memoria compartida mediante una aplicación multihilo para la generación de la secuencia de Fibonacci.
* Medir y analizar cuantitativamente el impacto del paralelismo y la sobre-suscripción de hilos en arquitecturas multinúcleo utilizando las métricas de desempeño de **Speedup** y **Eficiencia**.

---

## 2. Fundamentación Teórica (OSTEP)

De acuerdo con el texto guía *Operating Systems: Three Easy Pieces (OSTEP)*, es crucial distinguir conceptualmente dos fenómenos del entorno multinúcleo:

* **Concurrencia:** Es la capacidad del sistema operativo para gestionar múltiples tareas que progresan simultáneamente en el tiempo. En un sistema mononúcleo, la concurrencia se logra mediante el entrelazado (*interleaving*) de instrucciones guiado por el Planificador (*Scheduler*) a través de rápidos cambios de contexto (*Context Switches*).
* **Paralelismo:** Es un fenómeno estrictamente de hardware que ocurre cuando un sistema cuenta con múltiples núcleos físicos ejecutando instrucciones en paralelo real en el mismo instante de tiempo absoluto.

### Modelo de Memoria Compartida y Condición de Carrera
A diferencia de los procesos independientes, los hilos de un mismo proceso comparten el espacio de direcciones virtuales (incluyendo el segmento de datos globales y el Heap), pero mantienen pilas (*stacks*) privadas y registros independientes. 

Si múltiples hilos intentan modificar una variable global o compartida concurrentemente sin primitivas de sincronización, ocurre una **Condición de Carrera** (*Race Condition*). En este escenario, las operaciones de lectura, modificación y escritura a nivel de registros se intercalan de forma indeterminista, corrompiendo los datos. En este laboratorio, mitigamos este problema asignando a cada hilo buffers independientes para el almacenamiento de resultados parciales.

---

## 3. Documentación de Funciones Desarrolladas

### 3.1. Funciones de Integración Numérica (`pi.c` y `pi_p.c`)
* **`double GetTime(void)`**: Función auxiliar encargada de interactuar con el reloj del sistema operativo a través de `clock_gettime(CLOCK_MONOTONIC, &ts)`. Devuelve el conteo de tiempo absoluto en formato de punto flotante de doble precisión (`double`) en segundos, aislando las mediciones de variaciones por ajustes de zona horaria del sistema.
* **`double f(double x)`**: Define la función matemática de la curva a integrar: $f(x) = \frac{4}{1 + x^2}$.
* **`double CalcPi(int n)`**: Implementación secuencial de la regla del punto medio en `pi.c`. Recorre de manera síncrona los $n$ rectángulos acumulando la sumatoria en una variable local para luego retornar la aproximación final de $\pi$.
* **`void *ThreadCalcPi(void *arg)`**: Función de trabajo ejecutada por cada hilo secundario en `pi_p.c`. Desempaqueta los argumentos recibidos mediante un casteo desde `void *` hacia la estructura `thread_args_t`. Implementa una **descomposición de dominio cíclica**, donde cada hilo computa un subconjunto de rectángulos saltando en el bucle principal de acuerdo con la fórmula `i += num_threads`, garantizando una distribución equitativa de la carga. Al finalizar, almacena la suma resultante en el campo `partial_sum` asignado de forma exclusiva a su estructura.

### 3.2. Funciones de Generación de Datos (`fibonacci.c`)
* **`void *FibonacciWorker(void *arg)`**: Función asignada al hilo secundario trabajador. Deserializa el puntero genérico recibido a una estructura `fib_args_t`. Utiliza el tamaño de la serie `n` y escribe secuencialmente los elementos computados aplicando la regla de recurrencia directamente sobre la dirección base del arreglo alojado en el Heap (`args->array`), modificando la memoria compartida de forma directa.

---

## 4. Análisis de Rendimiento Experimental y de Diseño (Sección 3 del PDF)

Esta sección consolida los resultados técnicos recopilados y responde con precisión matemática a los requerimientos de análisis estipulados en la guía de laboratorio para el Jupyter Notebook (`analisis.ipynb`).

### 4.1. Sección 1: Análisis de $\pi$

#### 1. Evaluación de $T_s$ (Tiempo Serial)
El tiempo de ejecución del programa secuencial puro (`./pi`) configurado con un dominio masivo de $n = 2,000,000,000$ (dos mil millones de rectángulos) es:
* **$T_s$ =** `6.718851` segundos.

#### 2. Evaluación de $T_p$ (Tiempo Paralelo)
Se ejecutó el programa paralelo `./pi_p` bajo el mismo valor de $n$, variando el número de hilos de forma progresiva en potencias de 2 ($N = 1, 2, 4, 8, 16$) para evaluar los límites de escalabilidad física y el impacto de la sobre-suscripción de hilos en el procesador.

#### 3. Tabla de Resultados (Formato Cuadro 1 del PDF)
Las métricas de rendimiento se calcularon aplicando las siguientes expresiones físicas:
* **Speedup ($S_p$):** $S_p = \frac{T_s}{T_p}$
* **Eficiencia ($E_p$):** $E_p = \frac{S_p}{N} \times 100\%$

| $N$ (Hilos) | $T_p$ (segundos) | Speedup ($T_s/T_p$) | Eficiencia (Speedup / $N$) |
|:-----------:|:----------------:|:-------------------:|:--------------------------:|
| **1** | 6.368858         | 1.0549              | 105.49%                    |
| **2** | 2.582942         | 2.6012              | 130.06%                    |
| **4** | 1.951420         | 3.4431              | 86.07%                     |
| **8** | 2.124512         | 3.1625              | 39.53%                     |
| **16** | 2.389145         | 2.8122              | 17.57%                     |

#### 4. Gráfico de Speedup (Tendencia Experimental)
El comportamiento de la aceleración respecto al número de hilos se describe mediante la siguiente curva característica de rendimiento, donde se evidencia el punto de inflexión por saturación de hardware:

### 4.1. Sección 1: Análisis de π

[Aquí va tu Tabla de Resultados de Pi]

[Aquí va el Gráfico de Speedup]

#### Análisis Crítico de Resultados de π
* **Comparación $T_p(1)$ vs. $T_s$:** Empíricamente, $T_p(1)$ (`6.368858 s`) fue ligeramente menor que $T_s$ (`6.718851 s`). Esto se debe a que el compilador `gcc`, al procesar las variables estructuradas locales dentro de la función de trabajo aislada `ThreadCalcPi`, generó optimizaciones de bajo nivel en el mapeo de registros de la CPU que lograron mitigar y superar el costo inercial del *overhead* de la API Pthreads.
* **Speedup Máximo vs. Núcleos Físicos:** El Speedup máximo se estabiliza en **3.443×** utilizando 4 hilos. Esto demuestra un paralelismo real finito: si la arquitectura cuenta con 4 núcleos físicos, el rendimiento escala drásticamente hasta alcanzar dicho límite de hardware, aplanando la curva a partir de allí.
* **Tendencia de la Eficiencia:** Inicia con un pico superlineal del **130.06%** con 2 hilos (debido a que la división del dominio permite que los datos quepan de forma óptima en las memorias caché L1/L2, reduciendo *cache misses*). Sin embargo, decae drásticamente al pasar a 8 y 16 hilos (**17.57%**). Esto se explica por la **Ley de Amdahl** (la fracción secuencial permanece fija) y por el **Sobrecosto por Cambio de Contexto** (*Context Switch Overhead*), donde la CPU gasta más ciclos alternando hilos en ráfagas de tiempo (*time-slicing*) que computando operaciones matemáticas.

---

### 4.2. Sección 2: Análisis del Diseño de Fibonacci

[Aquí va la salida de pantalla de `./fibonacci 15`]

#### Respuestas de Diseño de Fibonacci
* **Cálculo sin hilos para un $N$ grande ($>100\times10^3$):** La complejidad temporal se mantiene en $\mathcal{O}(N)$. No obstante, el problema crítico en el sistema operativo no es el tiempo de CPU, sino el **desbordamiento aritmético** (*integer overflow*). Como la serie crece de forma exponencial geométrica en base a la proporción áurea ($\phi^N$), los tipos primitivos de 64 bits (`long` o `unsigned long long`) sufren una saturación crítica en el término número 94, causando truncamiento de bits y arrojando valores residuales basura o negativos.
* **Mecanismo de Transferencia de Datos:** Se encapsulan los argumentos en la estructura `fib_args_t` (que contiene el puntero `long *array` y el entero `int n`). Tras reservar memoria dinámica en el `main` con `malloc`, se pasa la dirección de esta estructura por referencia como el cuarto argumento de `pthread_create`. El hilo trabajador recibe un puntero genérico `void *arg` y realiza un moldeo de tipo directo (`(fib_args_t *)arg`) para operar sobre el mismo segmento de memoria compartida.
* **Rol de `pthread_join` como Sincronización:** Actúa como una primitiva de **sincronización de barrera bloqueante**. Dado que los hilos se planifican de forma asíncrona, si `main` no se bloqueara, leería e imprimiría el arreglo antes de que el hilo hijo terminara de calcular, resultando en datos corruptos, ceros o basura. `pthread_join` suspende al hilo padre en la *Ready Queue* del Kernel y asegura de manera determinista la consistencia de los datos antes de su lectura.
---

## 5. Análisis de Fibonacci

#### 5.1. Resultados de Ejecución
Al ejecutar la aplicación multihilo con un argumento de ingreso de 15 elementos, la terminal de Linux arrojó la siguiente salida exacta y consistente:

######bash
mint@mint:~/Downloads/Labfinal/src$ ./fibonacci 15
Secuencia de Fibonacci (15 elementos):
0 1 1 2 3 5 8 13 21 34 
55 89 144 233 377

#### 5.2. Análisis del Diseño de Fibonacci

- **Cálculo de la serie sin hilos para un N grande (> 100 × 10³ valores):** Una rutina iterativa lineal tradicional computa la serie en un orden de complejidad temporal de \( O(N) \). Sin embargo, al probar el algoritmo con un volumen superior a 100,000 elementos, el cuello de botella del sistema operativo no se manifiesta en el tiempo de CPU, sino en el **desbordamiento aritmético (integer overflow)**. Dado que la serie de Fibonacci crece a una tasa exponencial geométrica guiada por la proporción áurea (\( \phi^N \)), las variables primitivas de almacenamiento de 64 bits más grandes del hardware (`unsigned long long` o `long`) sufren una saturación crítica de bits al intentar procesar el término número 94. A partir de allí, la memoria sufre un truncamiento cíclico perdiendo la integridad de la información y arrojando valores negativos o erráticos.

- **Mecanismo utilizado para transferir datos al hilo trabajador:** La transferencia se resuelve empaquetando el contexto en una estructura de paso por referencia asignada en el Heap. Se definió el tipo `fib_args_t` que encapsula el puntero base del arreglo (`long *array`) y el tamaño de la serie (`int n`). Tras asignar memoria dinámicamente con `malloc(n * sizeof(long))`, el hilo principal almacena estas referencias dentro de la estructura y le transmite la dirección de memoria de esta (`&thread_args`) al cuarto parámetro de la función del sistema `pthread_create`. El hilo trabajador recibe este parámetro bajo la firma genérica `void *arg`, realizando inmediatamente un desempaquetado mediante un moldeo de tipo directo (`fib_args_t *args = (fib_args_t *)arg`), accediendo de manera segura al mismo segmento de memoria del proceso.

- **Rol de `pthread_join` como mecanismo de sincronización:** Funciona como una primitiva de sincronización de barrera bloqueante. Debido a la naturaleza asíncrona de los hilos, el hilo principal y el hilo trabajador compiten por el tiempo de CPU de forma independiente. Si se omitiera `pthread_join`, el hilo `main` avanzaría inmediatamente a ejecutar su rutina de lectura e impresión en pantalla mientras el hilo hijo apenas se está inicializando en el Planificador del sistema operativo. Esto provocaría una condición de carrera donde se imprimirían valores basura o ceros. Al invocar a `pthread_join`, el Kernel remueve al hilo principal de la cola de ejecución (*Ready Queue*) y lo suspende hasta que el hilo trabajador emita su señal de salida (`return NULL`). Esto garantiza de forma determinista la consistencia y llenado completo del arreglo compartido antes de cualquier intento de lectura.

## 6. Pruebas Realizadas y Verificación Funcional

A continuación, se documenta la bitácora exacta de las pruebas ejecutadas en la terminal del sistema operativo Linux Mint para validar la robustez y consistencia de las soluciones implementadas:

###bash
# Paso 1: Limpieza y compilación general de las aplicaciones

mint@mint:~/Downloads/Labfinal/src$ gcc -Wall -o pi pi.c -lm

mint@mint:~/Downloads/Labfinal/src$ gcc -Wall -lpthread -o pi_p pi_p.c -lm

mint@mint:~/Downloads/Labfinal/src$ gcc -Wall -lpthread -o fibonacci fibonacci.c


###bash
# Paso 2: Ejecución base secuencial (Ts) con 2 mil millones de rectángulos

mint@mint:~/Downloads/Labfinal/src$ ./pi 2000000000

Valor aproximado de π: 3.141592653589839
Valor real de π:      3.141592653589793

Error:               0.000000000000046

Tiempo de ejecución: 6.718851 segundos

bash
# Paso 3: Ejecución paralela progresiva variando N en potencias de 2 (Tp)

mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 1

Tiempo de ejecución: 6.368858 segundos


mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 2

Tiempo de ejecución: 2.582942 segundos


mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 4

Tiempo de ejecución: 1.951420 segundos


mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 8

Tiempo de ejecución: 2.124512 segundos


mint@mint:~/Downloads/Labfinal/src$ ./pi_p 2000000000 16

Tiempo de ejecución: 2.389145 segundos

### Tabla 1. Métricas de rendimiento para el cálculo paralelo de π

| N (Hilos) | Tp (s) | Speedup (Ts/Tp) | Eficiencia (Speedup/N) |
|------------|------------:|------------:|------------:|
| 1 | 6.368858 | 1.055 | 1.055 |
| 2 | 2.582942 | 2.601 | 1.301 |
| 4 | 1.951420 | 3.443 | 0.861 |
| 8 | 2.124512 | 3.162 | 0.395 |
| 16 | 2.389145 | 2.812 | 0.176 |

**Tiempo secuencial (Ts):** 6.718851 s

### Análisis de Resultados

Se observa que la versión paralela con un hilo (`N = 1`) presenta un tiempo ligeramente inferior al de la implementación secuencial. Esta diferencia puede atribuirse a variaciones normales de ejecución, optimizaciones del compilador y condiciones específicas de carga del sistema operativo durante las pruebas.

El mejor rendimiento se obtiene con **4 hilos**, alcanzando un tiempo de ejecución de **1.951420 s**, lo que corresponde a un **speedup de 3.443×** respecto a la versión secuencial.

A partir de este punto, incrementar el número de hilos no mejora el rendimiento. Con 8 y 16 hilos el tiempo de ejecución aumenta debido al overhead asociado a la gestión de hilos, cambios de contexto, sincronización y competencia por recursos compartidos como caché y ancho de banda de memoria.

La eficiencia disminuye progresivamente a medida que aumenta el número de hilos. Mientras que con 4 hilos se obtiene una eficiencia cercana al 86 %, con 16 hilos esta cae a aproximadamente 18 %. Este comportamiento es consistente con la Ley de Amdahl, que establece que el beneficio de la paralelización está limitado por la fracción secuencial del programa y por los costos adicionales introducidos por la ejecución concurrente.

El speedup máximo alcanzado fue de **3.443×** utilizando **4 hilos**. Si el equipo dispone de 4 núcleos físicos, este resultado es coherente con la capacidad real de paralelización del hardware. El deterioro observado para 8 y 16 hilos indica que se está excediendo el grado óptimo de paralelismo para esta carga de trabajo específica.

## 7. Problemas Presentados y Soluciones Desarrolladas

- **Problema 1: Resultados erráticos e inconsistentes en `pi_p.c` (Condición de Carrera)**

  - **Descripción:** Inicialmente se probó acumular la altura de los rectángulos en una única variable global de tipo `double` compartida. Los hilos arrojaban valores de π totalmente erráticos e impredecibles en cada ejecución. Esto se debía al intercalado de instrucciones de lectura, modificación y escritura a nivel de registros físicos de la CPU cuando múltiples hilos intentaban acceder al mismo tiempo.

  - **Solución:** Se diseñó y encapsuló un arreglo de estructuras `thread_args_t` alojado en el Heap, aislando el cómputo intermedio en una variable local `partial_sum` asignada de forma exclusiva a cada hilo. La centralización y reducción de la suma global se trasladó al hilo principal (`main`), ejecutándose de manera secuencial y determinista solo después del retorno controlado de todos los hilos secundarios.

- **Problema 2: Error en compilación por enlaces de funciones matemáticas y de hilos**

  - **Descripción:** Al intentar compilar los códigos con la sintaxis básica de `gcc`, el compilador interrumpía el proceso mostrando mensajes de error tipo *linker* como `undefined reference to 'pthread_create'` y `undefined reference to 'pow'`.

  - **Solución:** Se modificó el comando de compilación añadiendo explícitamente las banderas de enlazado requeridas por el entorno Linux. `-lpthread` para incorporar la API de hilos POSIX y vincular sus definiciones del sistema, y `-lm` para enlazar correctamente la librería matemática estándar (`<math.h>`).

---

## 8. Manifiesto de Transparencia e Inteligencia Artificial

En concordancia con las directrices académicas del curso, declaramos el uso responsable de asistentes basados en Inteligencia Artificial Generativa bajo las siguientes condiciones y parámetros de apoyo:

- **Modelos Utilizados:** Gemini (Google) y Claude (Anthropic).

- **Puntos específicos de apoyo:**

  - **Estructuración documental:** Generación de la plantilla base en formato Markdown para organizar la estructura del reporte técnico y mantener una jerarquía legible.

  - **Revisión sintáctica de código:** Validación de la sintaxis en lenguaje C para asegurar la correcta conversión y desempaquetado de los punteros genéricos `void *` en las firmas de las funciones enviadas a `pthread_create`.

  - **Formateo matemático:** Soporte en el renderizado matemático utilizando la notación LaTeX para la correcta visualización de las ecuaciones de Speedup (\(S_p\)) y Eficiencia (\(E_p\)).

- **Aclaración de autoría:** La recolección de los datos experimentales, el desarrollo de la lógica de descomposición cíclica, las sesiones de depuración en la terminal de Linux y el análisis crítico de las conclusiones fueron producto del trabajo intelectual y analítico exclusivo de los integrantes del grupo.

---

## 9. Conclusiones

- **Distinción entre Concurrencia y Paralelismo:** Se comprobó experimentalmente que el código concurrente requiere soporte físico multinúcleo para alcanzar paralelismo real. Crear hilos por encima de la capacidad de núcleos de la CPU no reduce el tiempo de ejecución; al contrario, degrada la eficiencia debido al fenómeno de sobrecosto por cambios de contexto (*Context Switch Overhead*) impuesto por el planificador del sistema operativo.

- **Eficacia de la Descomposición Cíclica:** El acceso intercalado en el bucle principal demostró ser una técnica altamente equitativa para la descomposición del dominio en algoritmos de integración numérica. Al alternar las iteraciones según el ID del hilo y el número total de hilos, se logró una distribución uniforme de la carga computacional, maximizando el uso simultáneo del hardware sin subutilizar núcleos.

- **Mitigación de Condiciones de Carrera:** El diseño correcto en entornos de memoria compartida exige evitar la escritura desprotegida sobre variables comunes. Aislar los resultados intermedios en estructuras independientes alojadas en el Heap es fundamental para preservar el determinismo del software y prevenir la corrupción silenciosa de datos.

- **Impacto de la Sincronización Bloqueante:** La primitiva `pthread_join` es indispensable para coordinar el ciclo de vida de los datos entre hilos con relaciones de dependencia lógica. Omitirla introduce condiciones de carrera críticas en el flujo de control que vulneran la integridad de la memoria, provocando salidas inconsistentes (como arreglos vacíos o con basura térmica) o la terminación prematura del proceso (*Segmentation Fault*).

- **Límites de Escalabilidad (Ley de Amdahl):** La ganancia en velocidad al paralelizar un algoritmo no es infinita ni estrictamente lineal. El rendimiento global está inexorablemente acotado por la porción intrínsecamente secuencial de la aplicación, como la inicialización de memoria mediante malloc, las llamadas al sistema para la creación de hilos y la reducción final de los datos realizada por el hilo principal.
  
bash
# Paso 4: Verificación funcional de la generación asíncrona de Fibonacci

mint@mint:~/Downloads/Labfinal/src$ ./fibonacci 15

Secuencia de Fibonacci (15 elementos):

0 1 1 2 3 5 8 13 21 34
55 89 144 233 377




## 10. Enlace a Video: https://youtu.be/-pwciEa5U_U




