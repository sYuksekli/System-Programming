#ifndef HELPER_H_
#define HELPER_H_ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


struct florist
{
	char* name;					// Name of the florist
	float x;					// x coordinate of the florist
	float y;					// y coordinate of the florist
	float speed;				// speed of the selling 
	char** flowerTypes;			// All the flower kinds that the florist sells
	int size; 					// The total number of flower kinds that florist sells
};


struct Queue
{
	char** request;		// If florist is busy, collect all the order and put them in a request array
	int offset; 		// Represents the total number of flower kinds that florist sells
	float* distance;	// Represents the last index that is used for request array
	char** client_no;	// Represents the id of client that the florist sells flower to
};

struct Statistic		// Threads returns a struct of statistics
{
	float total_time;	// Represents the total time that florist spends
	int totalOrder;		// Represents the total number of orders that is delivered
};


// to parse command line arguments
int parseCommandLine(int _argc, char* _argv[], char* file);

// To find distance between client and florist
float findChebyshevDistance(float client_x, float client_y, float florist_x, float florist_y);

// Print messages about florist
void florist_hasDelivered(char* _name, char* _flower, char* clientNum, float _time);
void florist_Closing(char* _name);

// To print statistics for every florists
void printStatics(struct Statistic** tmp, struct florist* _florist, int count);



#endif