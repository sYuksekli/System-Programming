all: program.o helper.o
	gcc -Wall program.o helper.o -lpthread -o program

program.o: program.c
	gcc -c -Wall program.c -std=gnu99 

helper.o: helper.c
	gcc -c -Wall helper.c -std=gnu99

clean:
	rm -rf *o