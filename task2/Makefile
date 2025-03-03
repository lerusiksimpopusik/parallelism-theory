CFLAG ?= -fopenmp
MATRIX_SIZE ?= 20000
NTHREADS ?= 1

task1: task1.c FORCE
	gcc -DMATRIX_SIZE=$(MATRIX_SIZE) -DNTHREADS=$(NTHREADS) $(CFLAG) -o $@ $<

task2: task2.c FORCE
	gcc -o $@ $< -DNTHREADS=$(NTHREADS) $(CFLAG) -lm


task3_each_section: task3_metod_1.cpp FORCE
	g++ -DMATRIX_SIZE=$(MATRIX_SIZE) -DNTHREADS=$(NTHREADS) $(CFLAG) -o $@ $<

task3_one_section: task3_metod_2.cpp FORCE
	g++ -DMATRIX_SIZE=$(MATRIX_SIZE) -DNTHREADS=$(NTHREADS) $(CFLAG) -o $@ $<	

FORCE:

