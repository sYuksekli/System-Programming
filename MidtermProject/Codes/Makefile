all: program.o cook.o student.o helper.o
	gcc -Wall program.o helper.o -lpthread -lrt -o program
	gcc -Wall cook.o helper.o -lpthread -lrt -o cook
	gcc -Wall student.o helper.o -lpthread -lrt -o student

program.o: program.c
	gcc -c -Wall program.c -std=gnu99 

cook.o: cook.c
	gcc -c -Wall cook.c -std=gnu99

student.o: student.c
	gcc -c -Wall student.c -std=gnu99

helper.o: helper.c
	gcc -c -Wall helper.c -std=gnu99

clean:
	rm -rf *o