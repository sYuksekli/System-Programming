#ifndef HELPER_H_
#define HELPER_H_ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



union semun {

	int val;
	struct semid_ds* buf;
	unsigned short* array;
};


// to parse command line arguments
int parseCommandLine(int _argc, char* _argv[], char* file);



// To print messages about state of the wholesaler
void wholesaler_isWaiting();
void wholesaler_Delivering(char* ing1, char* ing2);
void wholesaler_Obtained();



// To print messages about state of the chefs
void cook_isWaiting(int order, char* ing1, char* ing2);
void cook_hasTaken(int order, char* ing);
void cook_isPreparing(int order);
void cook_hasDelivered(int order);




#endif