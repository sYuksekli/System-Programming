all: server.o client.o helper.o
	gcc -Wall server.o helper.o -lpthread -lrt -o server
	gcc -Wall client.o helper.o -lpthread -lrt -o client
server.o: server.c
	gcc -c -Wall server.c -g -std=gnu99 
client.o: client.c
	gcc -c -Wall client.c -g -std=gnu99 
helper.o: helper.c
	gcc -c -Wall helper.c -g -std=gnu99
clean:
	rm -rf *o