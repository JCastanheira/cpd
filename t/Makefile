all: maxsat-serial maxsat-omp

maxsat-serial: maxsat-serial.c
	gcc maxsat-serial.c -o maxsat-serial -fopenmp -lm

maxsat-omp: maxsat-omp.c
	gcc maxsat-omp.c -o maxsat-omp -fopenmp -lm
