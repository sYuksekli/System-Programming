all: floristApp.o helper.o
	gcc -Wall floristApp.o helper.o -lpthread -lrt -o floristApp
floristApp.o: floristApp.c
	gcc -c -Wall floristApp.c -g -std=gnu99 
helper.o: helper.c
	gcc -c -Wall helper.c -g -std=gnu99
clean:
	rm -rf *o