target:
	clear
	gcc -o chart main.c bmp_create.c -lm -Wall -fopenmp