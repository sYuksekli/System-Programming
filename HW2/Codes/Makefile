all: program.o program2.o calculations.o
	gcc -Wall program.o calculations.o -lm -o program
	gcc -Wall program2.o calculations.o -lm -o program2

program.o: program.c
	gcc -c -Wall program.c -std=gnu99 

program2.o: program2.c
	gcc -c -Wall  program2.c -std=gnu99

calculations.o: calculations.c 
	gcc -c -Wall calculations.c -std=gnu99

clean:
	rm -rf *o