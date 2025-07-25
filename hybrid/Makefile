CC = gcc
MPICC = mpicc

CFLAGS = -fopenmp
LFILES = metrics.c

all: opt_omp opt_mpi opt_hybrid

opt_omp: opt_omp.c metrics.c
	$(CC) $(CFLAGS) opt_omp.c $(LFILES) -o opt_omp

opt_mpi: opt_mpi.c metrics.c
	$(MPICC) opt_mpi.c $(LFILES) -o opt_mpi

opt_hybrid: opt_hybrid.c metrics.c
	$(MPICC) -fopenmp opt_hybrid.c $(LFILES) -o opt_hybrid

clean:
	rm -f opt_omp opt_mpi opt_hybrid

reset:
	echo "method,prompt_index,time,total_time,avg_time" > results.csv

