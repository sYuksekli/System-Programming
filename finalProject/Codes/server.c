#include "helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>


// It holds the addresses of pointers to use in free operation
void** addrs;
// If addrs pointer is allocated, i change this variable 
int allocated = 0;
// to check if the input file is opened
int opened = 0;
// to check if the output file is opened
int opened2 = 0;
// to check if the threads are created
int checkPoint = 0;
// size of the addrs pointer
int count = 0;
// to check if the structs are created
int checkStruct = 0;
// file descriptor of the input file
int fd;
// file descriptor of the output file
int fd2;
// maximum number of threads 
int* x;
// number of threads at the beginning
int* s;
// signal set for sigint
sigset_t set;

// When signal received, this variable changed to one so that threads can understand that they need to exit
static volatile int signalReceived = 0;

// Graph data structure
struct Graph* graph;
// Cache structure
struct cacheList* cache;
// fullness of the cache
int* cacheSize;
// capacity of the cache
int* cacheSpace;

// Parameter for thread function
void** param;
// Thread ppol
pthread_t* threadIds;
// id of the resizer thread
pthread_t resizerThread;

// file descriptor of the socket
int socket_fd;
// Queue data structure
struct Queue* queue;

// Number of the threads that is busy
int* busyCount;
// mutex between main thread, resizer thread and pool of the threads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// conditional variable to inform pool of threads that a new connection arrives
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// conditional variable to inform main thread if all the threads are busy
pthread_cond_t cond_busy = PTHREAD_COND_INITIALIZER;
// conditional variable to inform resizer thread if all %75 of the pool is busy
pthread_cond_t cond_resize = PTHREAD_COND_INITIALIZER;

// mutex between pool of the threads to protect cache structure
pthread_mutex_t mutexCache = PTHREAD_MUTEX_INITIALIZER;
// conditional variable to inform reader threads
pthread_cond_t okToRead = PTHREAD_COND_INITIALIZER;
// conditional variable to inform writer threads
pthread_cond_t okToWrite = PTHREAD_COND_INITIALIZER;

// Variables for Readers-Writers paradigm
int* AR;
int* AW;
int* WR;
int* WW;



/* Prevent double inclusion */
/* Bit-mask values for 'flags' argument of becomeDaemon() */
#define BD_NO_CHDIR 01
/* Don't chdir("/") */
#define BD_NO_CLOSE_FILES 02
/* Don't close all open files */
#define BD_NO_REOPEN_STD_FDS 04
/* Don't reopen stdin, stdout, and stderr to
*
/dev/null */
#define BD_NO_UMASK0 010 /* Don't do a umask(0) */
#define BD_MAX_CLOSE 8192 /* Maximum file descriptors to close if
* sysconf(_SC_OPEN_MAX) is indeterminate */



// Makes the process daemon 
int becomeDaemon(int flags);

// Free resources
void exitGracefully();
// to parse command line arguments
void parseCommandLine(int _argc, char* _argv[], char* infile, int* _port, char* outfile, int* _s, int* _x);
// To create a critical section, blocking and unblocking operations
void begOfCriticalSection();
void endOfCriticalSection();
// Checking error case
void errorCheck(int res);
// SIGINT handler
void handler(int signo);
// Function for pool of the threads
void* handle_connection(void* param);
// Function for resizer thread
void* resize(void* parameters);


int main(int argc, char *argv[])
{

	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &handler;

	// To catch SIGINT
	if (sigaction(SIGINT,&sa,NULL) == -1)
	{
		perror("sigaction error is occured");
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(fd2,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	// I block SIGINT in some cases like freeing resources
	if (sigemptyset(&set) == -1 || sigaddset(&set,SIGINT) == -1)
	{
		perror("Failed to initialize signal mask");
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(fd2,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	// Create a file with the O_EXCL flag, so that if user starts server process again while the first one is running,
	// exit the program 
	int fd_shared = shm_open("prevent_two_instantiation",O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd_shared == -1)
	{
		exit(1);
	}


	// addrs holds the addresses for free operation
	begOfCriticalSection();
	addrs = (void**) malloc(sizeof(void*)*1000);
	allocated = 1;
	endOfCriticalSection();

	// Allocate space for command line arguments
	begOfCriticalSection();
	x = (int*) malloc(sizeof(int)*1);
	addrs[count] = x;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	s = (int*) malloc(sizeof(int)*1);
	addrs[count] = s;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	int* port = (int*) malloc(sizeof(int)*1);
	addrs[count] = port;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	char* inputfile = (char*) malloc(sizeof(char)*1000);
	addrs[count] = inputfile;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	char* outputfile = (char*) malloc(sizeof(char)*1000);
	addrs[count] = outputfile;
	++count;
	endOfCriticalSection();

	// Parse command line arguments
	parseCommandLine(argc, argv, inputfile, port, outputfile, s, x);


	// Open the input file to read it
	begOfCriticalSection();
	fd2 = openOutFile(outputfile);
	if (fd2 == -1)
	{
		perror("Failed to open output file");
		exitGracefully();
	}
	opened2 = 1;
	endOfCriticalSection();


	becomeDaemon(BD_NO_CHDIR | BD_NO_CLOSE_FILES | BD_NO_REOPEN_STD_FDS | BD_NO_UMASK0 | BD_MAX_CLOSE);


	// Printing to log file
	server_Executing(fd2, inputfile, outputfile, port, s, x);

	// Allocate space for number of busy threads
	begOfCriticalSection();
	busyCount = (int*) malloc(sizeof(int)*1);
	addrs[count] = busyCount;
	++count;
	endOfCriticalSection();

	// if it is zero, all threads are available
	*busyCount = 0;	

	// Aloocate space for variables for readers-writers 
	begOfCriticalSection();
	AR = (int*) malloc(sizeof(int)*1);
	addrs[count] = AR;
	++count;
	AW = (int*) malloc(sizeof(int)*1);
	addrs[count] = AW;
	++count;
	WR = (int*) malloc(sizeof(int)*1);
	addrs[count] = WR;
	++count;
	WW = (int*) malloc(sizeof(int)*1);
	addrs[count] = WW;
	++count;
	endOfCriticalSection();

	// Set their values
	*AR = 0;
	*AW = 0;
	*WR = 0;
	*WW = 0;

	

	// Open the input file to read it
	begOfCriticalSection();
	fd = openFile(inputfile);
	if (fd == -1)
	{
		write(fd2,"Failed to open input file\n",27);
		exitGracefully();
	}
	opened = 1;
	endOfCriticalSection();

	// Find the size of the input file to allocate space for buffer
	int sz = findSizeOfFile(fd);

	// Allocate space for buffer
	begOfCriticalSection();
	char* buf = (char*) malloc(sizeof(char)*(sz+1));
	addrs[count] = buf;
	++count;
	endOfCriticalSection();

	// Read the input file into the buf variable
	int res = readFile(fd,buf,sz);
	if (res == -1)
	{
		write(fd2,"Failed to read input file\n",27);
		exitGracefully();
	}

	// Close input file, I don't need this anymore.
	begOfCriticalSection();
	if (close(fd) == -1)
	{
		write(fd2,"Failed to close file descriptor\n",33);
		exitGracefully();
	}
	opened = 0;
	endOfCriticalSection();


	struct timeval t1;
	struct timeval t2;

	// Create graph
	begOfCriticalSection();
	graph = (struct Graph*) malloc(sizeof(struct Graph)*1);
	createGraph(graph, buf, sz, fd2);

	// Printing to log file
	server_loadingGraph(fd2);
	// Convert file to a graph structure
	gettimeofday(&t1,0); 		
 	createGraphfromFile(graph,buf,sz,fd2);
 	gettimeofday(&t2,0); 

 	// buf is freed
 	--count;
  	checkStruct = 1;
  	endOfCriticalSection();

  	// Calculate total time spending while waiting for an answer
	double elapsedTime = t2.tv_sec + t2.tv_usec / 1e6 - t1.tv_sec - t1.tv_usec / 1e6;
	// Print to log file
	server_graphLoaded(fd2, elapsedTime, graph->numOfVertices, graph->numOfEdges);


  	// Create queue. When a connection is established, its file descriptor is pushed into the queue.
  	// Thread of the pool takes the file descriptors from queue
	begOfCriticalSection();
	queue = (struct Queue*) malloc(sizeof(struct Queue)*1);
	createQueue(queue,1000,fd2);
	checkStruct = 2;
	endOfCriticalSection();


	// Create cache structure 
	begOfCriticalSection();
	cacheSpace = (int*) malloc(sizeof(int)*1);
	*cacheSpace = 100000;
	cache = (struct cacheList*) malloc(sizeof(struct cacheList)*(*cacheSpace));
	for (int i = 0; i < *cacheSpace; ++i)
	{
		cache[i].graphPath = (char*) malloc(sizeof(char)*10000);
	}
	cacheSize = (int*) malloc(sizeof(int)*1);
	*cacheSize = 0;
	checkStruct = 3;
	endOfCriticalSection();


	// To understand how many threads are created. 
	// If an error occurs, ı can cancel threads by this way.
	begOfCriticalSection();
	int* created = (int*) malloc(sizeof(int)*(*s)); 
	addrs[count] = created;
	++count;
	endOfCriticalSection();

	for (int i = 0; i < *s; ++i)
		created[i] = 0;

	// Create threads
	begOfCriticalSection();
	threadIds = (pthread_t*) malloc(sizeof(pthread_t)*(*x));
	addrs[count] = threadIds;
	++count;
	endOfCriticalSection();
	

	begOfCriticalSection();
	// Parameters to use in creating threads
	// Indicates florist index
	param = (void**) malloc(sizeof(void*)*(*x));
	for (int i = 0; i < *x; ++i)
	{
		param[i] = (void*) malloc(sizeof(void)*1);
		addrs[count] = param[i];
		++count;
	}
	addrs[count] = param;
	++count;
	endOfCriticalSection();

	// Set their values
	for (int i = 0; i < *x; ++i)
	{
		*((int*) ((int**) param)[i])  = i;	
	}


	begOfCriticalSection();
	for (int i = 0; i < *s; ++i)
	{	

		if (pthread_create(&threadIds[i],NULL,handle_connection,param[i]) != 0)
		{
			write(fd2,"Failed to create thread pool\n",30);
			for (int j = 0; j < *s; ++j)
			{
				if (created[j] == 1)
					pthread_cancel(threadIds[j]);
			}

			for (int j = 0; j < *s; ++j)
			{
				if (created[j] == 1)
					pthread_join(threadIds[j],NULL);
			}
			exitGracefully();
		}
		created[i] = 1;
	}
	checkPoint = 1;

	// Create resizer thread
	if (pthread_create(&resizerThread,NULL,resize,NULL) != 0) {
		write(fd2,"Failed to create resizer thread\n",33);
		exitGracefully();
	}
	checkPoint = 2;
	endOfCriticalSection();


	// Write to log file
	server_threadCreated(fd2, s);


	// Create socket
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	errorCheck(socket_fd);
	begOfCriticalSection();
	checkPoint = 3;
	endOfCriticalSection();


	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port = htons(*port);
	int _res = bind(socket_fd, (struct sockaddr*) &_addr, sizeof(_addr));
	errorCheck(_res);

	_res = listen(socket_fd,100);
	errorCheck(_res);

	while(1) {

		int newSocket_fd;
		struct sockaddr_in newAddr;
		socklen_t lenNewAddr = sizeof(newAddr);
		
		// If signal arrives, exit
		if (signalReceived == 1)
		{
			break;
		}
		// Accept the connection
		newSocket_fd = accept(socket_fd, (struct sockaddr*) &newAddr, &lenNewAddr);
		// If signal arrives, exit
		if (newSocket_fd == -1 && errno == EINTR && signalReceived == 1)
		{	
			break;
		}

		// Control the returned value from accept
		errorCheck(newSocket_fd);

		// Queue is common, so get the mutex
		pthread_mutex_lock(&mutex);
		// If all threads are busy, wait
		while(*busyCount == *s) {
			server_noThreadAvailable(fd2);
			pthread_cond_wait(&cond_busy,&mutex);
		}

		// If queue is full, make a reallocation
		if (queue->size >= queue->capacity)
		{	
			queue->capacity *= 2;
			int* queue_tmp;
			queue_tmp = (int*) realloc(queue->array,queue->capacity*sizeof(int));
			if (queue_tmp == NULL)
			{
				write(fd2,"Reallocation failed. Memory is not enough\n",43);
				pthread_mutex_unlock(&mutex);
				exitGracefully();
			}
			queue->array = queue_tmp;
		}

		// Add file descriptor into the queue
		addElement(queue,newSocket_fd);
		// Wake a thread that waits a file descriptor
		pthread_cond_signal(&cond);
		// Release the lock
		pthread_mutex_unlock(&mutex);
	}

	return 0;
}



void* handle_connection(void* parameters)
{

	int ind = *((int*) parameters); 

	int result, client_fd;  
	char* request;
	char* ans;
	int* _size;
	int* _path;
	while(1) {

		// Get the lock and wait for a file descriptor
		pthread_mutex_lock(&mutex);
		server_threadWaitingConnection(fd2, ind);
		while(isEmpty(queue) == 1 && !signalReceived) {
			pthread_cond_wait(&cond,&mutex);
		}

		if (signalReceived == 1)
		{	
			pthread_mutex_unlock(&mutex);
			break;
		}

		// Remove the file descriptor to send message to client
		client_fd = removeFront(queue);
		// Current thread starts working, increase the number of busy threads
		++ *busyCount;

		double sysLoad = (double) *busyCount / *s;
		sysLoad *= 100.0;
		// Write message to log file
		server_connectionDelegated(fd2, ind, sysLoad);

		// Send signal to resizer thread, so that it can check whether it should increase the number of threads or not
		pthread_cond_signal(&cond_resize);
		// Release the lock
		pthread_mutex_unlock(&mutex);

		// If a path is available, it is copied into _path arrauy
		// _size is the size of the _path array
		_size = (int*) malloc(sizeof(int)*1);
		_path = (int*) malloc(sizeof(int)*graph->numOfVertices);

		// Allocate space for request array
		request = (char*) malloc(sizeof(char)*1000);
		char* temp = request;
		// Read the requested path as string from socket that is send by client
		read(client_fd,request,1000);

		// Parse the destination and source nodes 
		char* token = strtok_r(request,"-",&request);
		int src = atoi(token);

		char* token2 = strtok_r(request,"\n",&request);
		int dst = atoi(token2);

		if (src >= graph->numOfVertices)
		{
			write(fd2,"There is no vertex with this id. Please enter valid index\n",59);
			if (shm_unlink("prevent_two_instantiation") == -1)
			{
				write(fd2,"Failed to unlink shared memory\n",32);
				exit(1);
			}
			exit(1);
		}

		// Allocate space for answer that will send by server to client
		ans = (char*) malloc(sizeof(char)*10000);
		char tmp[1000];

		// For readers 
		pthread_mutex_lock(&mutexCache);
		while ((*AW + *WW) > 0) {
			*WR += 1;
			pthread_cond_wait(&okToRead,&mutexCache);
			*WR -= 1;
		}

		*AR += 1;
		pthread_mutex_unlock(&mutexCache);
		// Write to log file
		server_threadSearchingDatabase(fd2, ind, src, dst);
		// Check if an requested path is already exists in the cache, if it is present, path is copied into ans string
		int check = checkPastCalculations(cache, cacheSize, ans, src, dst);
		pthread_mutex_lock(&mutexCache);

		*AR -= 1;
		if (*AR == 0 && *WW > 0)
			pthread_cond_signal(&okToWrite);
		pthread_mutex_unlock(&mutexCache);

		if (check == 1)
		{
			// Print to log file
			server_pathFoundInCache(fd2, ind, ans);
		}
		
		// If check is zero, it means requested path is not present in cache.
		if (check == 0)
		{	
			// Write to log file 
			server_noPathInDatabase(fd2, ind, src, dst);
			// Search the path if it is available
			result = findPath_usingBFS(_path,_size,src,dst,graph,fd2);
			// If result is zero, there is no available path between these nodes
			if (result == 0) {

				// Write to log file
				server_pathNotPossible(fd2, ind, src, dst);
				sprintf(ans,"path not possible from node %d to %d\n",src,dst);
			}

			// If it is one, convert the path array to string
			else {

				sprintf(ans,"%d - ",_path[0]);
				for (int i = 1; i < *_size-1; ++i)
				{
					sprintf(tmp,"%d - ",_path[i]);
					strcat(ans,tmp);
				}
				sprintf(tmp,"%d ",_path[*_size-1]);
				strcat(ans,tmp);

				// Write to lof file
				server_pathCalculated(fd2, ind, ans);

				struct Edge _edge;
				_edge.source = src;
				_edge.dest = dst;

				// For writers
				pthread_mutex_lock(&mutexCache);
				while ((*AW + *AR) > 0) {   
					*WW += 1;
					pthread_cond_wait(&okToWrite, &mutexCache);
					*WW -= 1;
				}
				
				*AW += 1;
				pthread_mutex_unlock(&mutexCache);	
				// Check one more time if the requested path is exists in cache to avoid writing same path two times
				int present = isPresent(cache, cacheSize, src, dst);
				// If it is zero, write the path into the cache
				if (present == 0)
				{
					// If size of the cache reachs the capacity, make reallocation
					if (*cacheSize >= *cacheSpace)
					{	
						*cacheSpace *= 2;
						struct cacheList* tmp_cache;
						tmp_cache = (struct cacheList*) realloc(cache,(*cacheSpace)*sizeof(struct cacheList));
						if (tmp_cache == NULL)
						{
							write(fd2,"Cache can not be extended. Memory is not enough\n",49);
							if (shm_unlink("prevent_two_instantiation") == -1)
							{
								write(fd2,"Failed to unlink shared memory\n",32);
								exit(1);
							}
							exit(1);
						}

						cache = tmp_cache;
						for (int i = *cacheSize; i < *cacheSpace; ++i)
						{
							cache[i].graphPath = (char*) malloc(sizeof(char)*10000);
						}
					}

					server_addingPathToCache(fd2, ind);
					// Save the information in cache
					cache[*cacheSize].edge = _edge;
					strcpy(cache[*cacheSize].graphPath,ans);
					// Increase the size of the cache
					++ *cacheSize;
				}
				
				pthread_mutex_lock(&mutexCache);
				*AW -= 1;

				if (*WW > 0)
					pthread_cond_signal(&okToWrite);
				else if (*WR > 0)
					pthread_cond_broadcast(&okToRead);
				pthread_mutex_unlock(&mutexCache);
			}
		}

		// Write the answer to socket
		write(client_fd,ans,10000);
		pthread_mutex_lock(&mutex);
		// Operation is done, decrease the number of busy threads
		-- *busyCount;
		// Signal to main threads if it waits for a available thread
		pthread_cond_signal(&cond_busy);

		// Writing to socket finished, close the descriptor
		close(client_fd);
		// free resources
		free(temp);
		free(ans);
		free(_path);
		free(_size);
		// Release the lock
		pthread_mutex_unlock(&mutex);

	}

	return NULL;
}

void* resize(void* parameters)
{	

	int th_count;
	int* c;

	while(1) {

		pthread_mutex_lock(&mutex);
		th_count = *s;
		// Wait for until busy threads reached the %75 of pool 
		while(*busyCount <= th_count*0.75 && !signalReceived) {
			pthread_cond_wait(&cond_resize,&mutex);
		}

		if (signalReceived == 1){
			pthread_mutex_unlock(&mutex);
			break;
		}

		// If number of the threads does not reach the maximum value, increase the number ıf threads
		if (*s < *x)
		{	
			if (th_count <= 4) {
				*s = th_count + 1;
			}

			else {
				*s = (int) th_count + th_count*0.25;
				if (*s > *x)
				{
					*s = *x;
				}
			}

			int sz = *s - th_count;
			c = (int*) malloc(sizeof(int)*sz); 
			for (int i = 0; i < sz; ++i)
				c[i] = 0;

			int param_index = th_count;
			for (int i = 0; i < sz; ++i)
			{
				if (pthread_create(&threadIds[i],NULL,handle_connection,param[param_index]) != 0)
				{	
					write(fd2,"Failed to extend the size of the pool\n",39);
					free(c);
					pthread_mutex_unlock(&mutex);
					if (shm_unlink("prevent_two_instantiation") == -1)
					{
						write(fd2,"Failed to unlink shared memory\n",32);
						exit(1);
					}
					exit(1);
				}
				c[i] = 1;
				param_index += 1; 
			}
			free(c);
			// Write to log file
			server_poolExtended(fd2, s);
		}

		// operation is done, release the lock
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

// To create critical section
void begOfCriticalSection()
{
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		write(fd2,"Blocking operation is failed\n",30);
		exitGracefully();
	}
}

void endOfCriticalSection()
{
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		write(fd2,"Unblocking operation is failed\n",32);
		exitGracefully();
	}
}


// Free resources
void exitGracefully()
{	

	begOfCriticalSection();
	if (opened == 1){
		close(fd);
		opened = 0;
	}

	if (opened2 == 1){
		close(fd2);
		opened2 = 0;
	}

	if (checkPoint == 1)
	{	
		signalReceived = 1;
		pthread_cond_broadcast(&cond);
		for (int i = 0; i < *s; ++i)
		{
			pthread_join(threadIds[i],NULL);
		}
	}

	if (checkPoint == 2)
	{	
		signalReceived = 1;
		pthread_cond_broadcast(&cond);
		pthread_cond_signal(&cond_resize);
		for (int i = 0; i < *s; ++i)
		{
			pthread_join(threadIds[i],NULL);
		}
		pthread_join(resizerThread,NULL);
	}

	if (checkPoint == 3)
	{	
		signalReceived = 1;
		pthread_cond_broadcast(&cond);
		pthread_cond_signal(&cond_resize);
		for (int i = 0; i < *s; ++i)
		{
			pthread_join(threadIds[i],NULL);
		}
		pthread_join(resizerThread,NULL);
		close(socket_fd);

		checkPoint = 0;
	}

	if (checkStruct == 1)
	{
		for (int i = 0; i < graph->numOfVertices; ++i)
			free(graph->edges[i].adjacents);
		free(graph->edges);
		free(graph);
	}

	if (checkStruct == 2)
	{
		for (int i = 0; i < graph->numOfVertices; ++i)
			free(graph->edges[i].adjacents);
		free(graph->edges);
		free(graph);

		free(queue->array);
		free(queue);
	}

	if (checkStruct == 3)
	{
		for (int i = 0; i < graph->numOfVertices; ++i)
			free(graph->edges[i].adjacents);
		free(graph->edges);
		free(graph);

		free(queue->array);
		free(queue);

		for (int i = 0; i < *cacheSpace; ++i)
			free(cache[i].graphPath);
		free(cache);
		free(cacheSize);
		free(cacheSpace);

		checkStruct = 0;
	}

	for (int i = count -1; i >= 0; --i){
		free(addrs[i]);
		--count;
	}

	if (allocated == 1)
		free(addrs);

	endOfCriticalSection();

	if (shm_unlink("prevent_two_instantiation") == -1)
	{
		write(fd2,"Failed to unlink shared memory\n",32);
		exit(1);
	}

	exit(1);
}

// Checking error case
void errorCheck(int res)
{
	if(res == -1){
		write(fd2,"Socket operation failed\n",25);
		exitGracefully();
	}
}


void handler(int signo)
{
	server_signalReceived(fd2);
	if (shm_unlink("prevent_two_instantiation") == -1)
	{
		write(fd2,"Failed to unlink shared memory\n",32);
		exit(1);
	}

	server_threadsTerminated(fd2);
	exit(1);
}



int becomeDaemon(int flags)
{

	int maxfd, fd;
	/* Become background process */
	switch (fork()) {

		case -1: return -1;

		case 0: break;

		default: _exit(EXIT_SUCCESS);
	}

	/* Become leader of new session, dissociate from tty */
	if (setsid() == -1)
		return -1;

	switch (fork()) {

		case -1: return -1;

		/* thanks to 2 nd fork, there is no way of acquiring a tty */	
		case 0: break;

		default: _exit(EXIT_SUCCESS);
	}

	if (!(flags & BD_NO_UMASK0))
		umask(0);

	/* Clear file mode creation mask */
	if (!(flags & BD_NO_CHDIR))
		chdir("/");

	/* Change to root directory */
	if (!(flags & BD_NO_CLOSE_FILES)) {
		/* Close all open files */
		maxfd = sysconf(_SC_OPEN_MAX);

		if (maxfd == -1)
			maxfd = BD_MAX_CLOSE;


		for (fd = 0; fd < maxfd; fd++){
			if (fd != fd2)
			{
				close(fd);
			}
		}
	}

	if (!(flags & BD_NO_REOPEN_STD_FDS)) {

		close(STDIN_FILENO);

		/* Reopen standard fd's to /dev/null */
		fd = open("/dev/null", O_RDWR);
		if (fd != STDIN_FILENO)
			return -1;
		/* 'fd' should be 0 */

		if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			return -1;

		if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -1;
	}


	return 0;
}


void parseCommandLine(int _argc, char* _argv[], char* infile, int* _port, char* outfile, int* _s, int* _x)
{

	int opt;
	int count = 0; // To check all the options are entered by the user.
	while((opt = getopt(_argc, _argv, ":i:p:o:s:x:")) != -1)
	{
		switch(opt)
		{

			case 'i':
			{
				strcpy(infile,optarg);
				++count;
				break;
			}

			case 'p':
			{
				*_port = atoi(optarg);
				++count;
				break;
			}
			case 'o':
			{
				strcpy(outfile,optarg);
				++count;
				break;
			}
			case 's':
			{	

				*_s = atoi(optarg);
				if (*_s < 2)
				{
					write(fd2,"s must be at least two\n",24);
					exitGracefully();
				}

				// To check if the user enters an integer number
				for (int i = 0; i < strlen(optarg); ++i)
				{
					if (optarg[i] < 48 || optarg[i] > 57)
					{
						write(fd2,"Please enter integer\n",22);
						exitGracefully();
					}
				}

				++count;
				break;
			}
			case 'x':
			{

				// To check if the user enters an integer number
				for (int i = 0; i < strlen(optarg); ++i)
				{
					if (optarg[i] < 48 || optarg[i] > 57)
					{
						write(fd2,"Please enter integer\n",22);
						exitGracefully();
					}
				}

				*_x = atoi(optarg);
				if (*_s > *_x)
				{
					write(fd2,"x indicates maximum number of threads. It must be greater or equal than s value\n",81);
					exitGracefully();
				}

				++count;
				break;
			}

			case ':':
			{
				write(fd2,"option needs a value\n",22);
				exitGracefully();
			}

			case '?':
			{
				char text[200];
				sprintf(text,"Unknown option %c,You must enter a command line like this : -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24\n",optopt);
				write(fd2,text,strlen(text));
				exitGracefully();
			}
		}
	}

	for ( ; optind < _argc; ++optind)
	{
		char text[300];
		sprintf(text,"You entered extra arguments %s,You must enter a command line like this : -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24\n",_argv[optind]);
		write(fd2,text,strlen(text));
		exitGracefully();
	}

	if (count != 5)
	{
		write(fd2,"You entered missing command lines, You must enter a command line like this : -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24\n",128);
		exitGracefully();
	}

}