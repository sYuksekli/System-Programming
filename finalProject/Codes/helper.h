#ifndef HELPER_H_
#define HELPER_H_ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>




// Queue data structure to use in bfs algorithm
struct Queue
{
	int* array;					// array to hold contents
	int size;					// fullness of the queue
	int capacity;				// capacity of the queue
};

// struct of Edge to represent edge between to vertices
struct Edge
{
	int dest;					// destination vertex
	int source;					// source vertex
};


// Cache data structure to make operations faster
struct cacheList
{
	struct Edge edge;			// Destination and source information
	char* graphPath;			// path informatin, saved as string
};


// list implementation to represent graph as adjacency list
struct list
{
	int vertexID; 				// current vertex id
	struct Edge* adjacents;		// holds the all neighbors as edges of the current vertex
	int size; 					// size of the adjacents array
	int capacity;				// capacity of the lsit
};

// Implementation of graph structure
struct Graph
{
	struct list* edges;			// array of list
	int numOfVertices; 			// number of nodes in the graph
	int numOfEdges; 			// number of edges in the graph
};





// Create and allocate space for queue 
void createQueue(struct Queue* _queue, int _capacity, int _fd);
// Add an element into the queue
void addElement(struct Queue* _queue, int element);
// Removes the first element from queue and returns the element 
int removeFront(struct Queue* _queue);
// Checks whether queue is empty
int isEmpty(struct Queue* _queue);


// To create a graph according to the values given as parameters
void createGraph(struct Graph* _graph, char* _buf, int bufSize, int _fd);
// Checks if an edge is present between source and destination given as parameter
int isEdge(struct Graph* _graph, int _source, int _dest);
// Adds an edge into the graph
int addEdge(struct Graph* _graph, struct Edge _edge);


// Reallocation for graph in case of insufficient memory
void reallocation(struct Edge** edges, int _newCap, int fd2);
// Finds the total number of comments in the file
int findNumOfComments(char* _buf,int buf_size, int _fd);
// Finds the total number of vertices in the graph
int findNumOfVertices(char* _buf,int buf_size, int _fd);
// Finds the total number of vertices in the graph
int findNumOfEdges(char* _buf,int buf_size, int _fd);
// Loads a graph from the given buffer 
void createGraphfromFile(struct Graph* graph, char* _buf, int bufSize, int _fd);


// To check if a path is already exists in the cache
int isPresent(struct cacheList* _cache, int* _size, int _source, int _dest);
// To check past calculation to avoid recalculation the same path
int checkPastCalculations(struct cacheList* _cache, int* _size, char* path, int _source, int _dest);
// Finds a path between given nodes if it is present and return one, if it isn't returns zero
int findPath_usingBFS(int* path, int* _count, int _source, int _dest, struct Graph* _graph, int _fd);


// find the number of characters in the file
int findSizeOfFile(int _fd);
// open the input file and return its file descriptor
int openFile(char* _filename);
// open the outğut file and return its file descriptor
int openOutFile(char* _filename);
// read the opened file and return one if the operatipon is successfull.
int readFile(int _fd, char* _buf, int buf_size);


// To print messages about the states of the client processes
void client_printConnecting(int _pid, char* _ip, int *_port, char* curTime);
void client_Connected(int _pid, int* _source, int* _dest, char* curTime);
void client_getAnswer(int _pid, char* _ans, double _time, char* curTime);


// To print messages about the states of the server process
void server_Executing(int _fd, char* input, char* output, int* _port, int* _s, int* _x);
void server_loadingGraph(int _fd);
void server_graphLoaded(int _fd, double elapsed_time, int _vertex, int _edges);
void server_threadCreated(int _fd, int* _s);
void server_threadWaitingConnection(int _fd, int _index);
void server_connectionDelegated(int _fd, int _index, double systemLoad);
void server_threadSearchingDatabase(int _fd, int _index, int _source, int _dest);
void server_noPathInDatabase(int _fd, int _index, int _source, int _dest);
void server_pathCalculated(int _fd, int _index, char* _ans);
void server_addingPathToCache(int _fd, int _index);
void server_pathNotPossible(int _fd, int _index, int _source, int _dest);
void server_pathFoundInCache(int _fd, int _index, char* _ans);
void server_poolExtended(int _fd, int* _num);
void server_noThreadAvailable(int _fd);
void server_signalReceived(int _fd);
void server_threadsTerminated(int _fd);




#endif