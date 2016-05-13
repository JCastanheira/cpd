all: maxsat-serial maxsat-mpi maxsat-omp

maxsat-serial: maxsat-serial.c
	gcc maxsat-serial.c -o maxsat-serial -fopenmp -lm

maxsat-mpi: maxsat-mpi.c
	mpicc -g -fopenmp maxsat-mpi.c -o maxsat-mpi -lm

maxsat-omp: maxsat-omp.c
	gcc maxsat-omp.c -o maxsat-omp -fopenmp -lm
