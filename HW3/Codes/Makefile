all: program.o
	gcc -Wall program.o -lm -o program

program.o: program.c
	gcc -c -Wall program.c -std=gnu99 
	
clean:
	rm -rf *o