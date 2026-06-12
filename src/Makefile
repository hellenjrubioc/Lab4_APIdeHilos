CC = gcc
CFLAGS = -O2 -Wall -std=c99
LM = -lm
LPTHREAD = -lpthread

TARGETS = pi_s pi_p fibonacci

.PHONY: all clean run_tests

all: $(TARGETS)

pi_s: pi.c
	$(CC) $(CFLAGS) -o pi_s pi.c $(LM)
	@echo "✓ pi_s compilado exitosamente"

pi_p: pi_p.c
	$(CC) $(CFLAGS) -o pi_p pi_p.c $(LPTHREAD) $(LM)
	@echo "✓ pi_p compilado exitosamente"

fibonacci: fibonacci.c
	$(CC) $(CFLAGS) -o fibonacci fibonacci.c $(LPTHREAD)
	@echo "✓ fibonacci compilado exitosamente"

clean:
	rm -f $(TARGETS) *.o
	@echo "Archivos compilados eliminados"

test: all
	@echo "\n=== Prueba de pi_s (n=100000000) ==="
	./pi_s 100000000
	@echo "\n=== Prueba de pi_p (n=100000000, 4 hilos) ==="
	./pi_p 100000000 4
	@echo "\n=== Prueba de fibonacci (15 números) ==="
	./fibonacci 15
	@echo "\n✓ Todas las pruebas completadas"

help:
	@echo "Targets disponibles:"
	@echo "  make all      - Compilar todos los programas"
	@echo "  make pi_s     - Compilar pi.c (versión serial)"
	@echo "  make pi_p     - Compilar pi_p.c (versión paralela)"
	@echo "  make fibonacci - Compilar fibonacci.c"
	@echo "  make clean    - Eliminar archivos compilados"
	@echo "  make test     - Compilar y ejecutar pruebas básicas"
	@echo "  make help     - Mostrar esta ayuda"
