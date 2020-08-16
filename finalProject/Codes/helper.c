#include "helper.h"



int findSizeOfFile(int _fd)
{
	int  buf_size = lseek(_fd,0,SEEK_END);
	return buf_size;
}

int openFile(char* _filename)
{
	// Open the input file in read only mode
	int fd = open(_filename,O_RDONLY);	
	return fd;	
}

int openOutFile(char* _filename)
{
	// Open the output file in write only mode
	int fd = open(_filename,O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);	
	return fd;	
}

int readFile(int _fd, char* _buf, int buf_size)
{			
	// Set the offset to beginning
	lseek(_fd,0,SEEK_SET);	
	// Read the file	,
	int size = 0;						
	while (size != buf_size)					
	{	
		size = read(_fd,_buf,buf_size);	
		if(size == -1)
		{
			return -1;
		}					
	}

	return buf_size;
}


/*    QUEUE FUNCTIONS    */
void createQueue(struct Queue* _queue, int _capacity, int _fd)
{
	// Allocate space according to the _capacity
	_queue->array = (int*) malloc(sizeof(int)*_capacity);
	if (_queue->array == NULL)
	{
		write(_fd,"Cannot allocate memory for queue in createQueue function\n",58);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}
	_queue->capacity = _capacity;
	_queue->size = 0;
}

void addElement(struct Queue* _queue, int element)
{
	// Add element to the end of the queue
	_queue->array[_queue->size] = element;
	// Increase the size of the queue
	_queue->size += 1;
}

int removeFront(struct Queue* _queue)
{
	// Take the first element
	int front = _queue->array[0];
	// Decrease the size
	-- _queue->size;
	// Shift the elements
	for (int i = 0; i < _queue->size ; ++i)
	{
		_queue->array[i] = _queue->array[i+1];
	}

	// return the first element
	return front;
}

int isEmpty(struct Queue* _queue)
{
	if (_queue->size == 0)
		return 1;

	return 0;
}


/*    GRAPH FUNCTIONS  */
void createGraph(struct Graph* _graph, char* _buf, int bufSize, int _fd)
{

	// Find the number of nodes in the file
	int _vertices = findNumOfVertices(_buf,bufSize, _fd);
	int _edges = findNumOfEdges(_buf, bufSize, _fd);

	_graph->numOfVertices = _vertices;
	_graph->numOfEdges = _edges;

	_graph->edges = (struct list*) malloc(sizeof(struct list)*_vertices);
	if (_graph->edges == NULL)
	{
		write(_fd,"Cannot allocate memory for graph in createGraph function\n",58);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	for (int i = 0; i < _vertices; ++i)
	{
		_graph->edges[i].adjacents = (struct Edge*) malloc(sizeof(struct Edge)*2000);
		if (_graph->edges[i].adjacents == NULL)
		{
			write(_fd,"Cannot allocate memory for graph in createGraph function\n",58);
			if (shm_unlink("prevent_two_instantiation") == -1)
			{
				write(_fd,"Failed to unlink shared memory\n",32);
				exit(1);
			}
			exit(1);
		}
		_graph->edges[i].capacity = 2000;
		_graph->edges[i].size = 0;
	}
}

int isEdge(struct Graph* _graph, int _source, int _dest)
{
	// Traverse the neighbors of the given vertex
	for (int i = 0; i < _graph->edges[_source].size; ++i)
	{
		if (_graph->edges[_source].adjacents[i].dest == _dest)
		{
			return 1;
		}
	}

	return 0;
}

int addEdge(struct Graph* _graph, struct Edge _edge)
{
	// If an edge is already exists, return zero
	if (isEdge(_graph,_edge.source,_edge.dest) == 1)
	{
		return 0;
	}

	// Otherwise, add an edge to given vertex and increase the size of the list
	_graph->edges[_edge.source].adjacents[_graph->edges[_edge.source].size] = _edge;
	_graph->edges[_edge.source].size += 1;

	return 1;
}



int findNumOfComments(char* _buf,int buf_size, int _fd)
{
	int comments = 0;

	char* token;
	char* bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	if (bufCpy == NULL)
	{
		write(_fd,"Cannot allocate memory in findNumOfComments function\n",54);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	char* temp = bufCpy;
	strcpy(bufCpy,_buf);

	token = strtok_r(bufCpy,"\n",&bufCpy);
	// Continue until token is null
	while(token[0] == '#')
	{
		++comments;
		// Go to the bottom line
		token = strtok_r(bufCpy,"\n",&bufCpy);
	}

	free(temp);
	return comments;
}

int findNumOfVertices(char* _buf, int buf_size, int _fd)
{
	// Initially, maximum value of the vertex id is zero
	int max = 0;

	int _source, _dest;
	char* token;
	char* bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	if (bufCpy == NULL)
	{
		write(_fd,"Cannot allocate memory in findNumOfVertices function\n",54);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	char* temp = bufCpy;
	strcpy(bufCpy,_buf);

	int comments = findNumOfComments(_buf,buf_size, _fd);
	comments += 1;

	// Skip the comments 
	for (int i = 0; i < comments; ++i)
	{
		token  = strtok_r(bufCpy,"\n",&bufCpy);
	}
	
	// Continue until token is null
	while(token != NULL)
	{
		
		// Parse source and the destination values
		char* token2 = strtok_r(token,"\t",&token);
		_source = atoi(token2);
		// If id is greater than max, new value of max is _source
		if (max < _source)
		{
			max = _source;
		}

		char* token3 = strtok_r(token,"\n",&token);
		// If id is greater than max, new value of max is _dest
		_dest = atoi(token3);
		if (max < _dest)
		{
			max = _dest;
		}

		// Go to the bottom line
		token = strtok_r(bufCpy,"\n",&bufCpy);
	}

	free(temp);
	max += 1;
	return max;
}

int findNumOfEdges(char* _buf,int buf_size, int _fd)
{
	
	int line = 0;

	char* token;
	char* bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	if (bufCpy == NULL)
	{
		write(_fd,"Cannot allocate memory in findNumOfEdges function\n",51);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	char* temp = bufCpy;
	strcpy(bufCpy,_buf);

	int comments = findNumOfComments(_buf,buf_size, _fd);
	comments += 1;

	// Skip the comments 
	for (int i = 0; i < comments; ++i)
	{
		token  = strtok_r(bufCpy,"\n",&bufCpy);
	}
	
	// Continue until token is null
	while(token != NULL)
	{
		++line;
		// Go to the bottom line
		token = strtok_r(bufCpy,"\n",&bufCpy);
	}

	free(temp);
	return line;
}

void reallocation(struct Edge** edges, int _newCap, int _fd)
{
	struct Edge* tmp_edge;
	if ( (tmp_edge = (struct Edge*) realloc(*edges, sizeof(struct Edge)*_newCap)) == NULL)
	{
		write(_fd,"Memory is not enough. Reallocation failed\n",43);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		free(tmp_edge);
		exit(1);
	}

	*edges = tmp_edge;
}

void createGraphfromFile(struct Graph* graph, char* _buf, int bufSize, int _fd)
{	

	int _source, _dest;
	char* temp = _buf;

	// Skip the comments
	char* token;

	int comments = findNumOfComments(_buf,bufSize, _fd);
	comments += 1;

	for (int i = 0; i < comments; ++i)
	{	
		token  = strtok_r(_buf,"\n",&_buf);
	}

	while(token != NULL)
	{	

		// Parse source and the destination values
		char* token2 = strtok_r(token,"\t",&token);
		_source = atoi(token2);
		char* token3 = strtok_r(token,"\n",&token);
		_dest = atoi(token3);

		// Create an adge 
		struct Edge _edge;
		_edge.source = _source;
		_edge.dest = _dest;

		// If number of the neighbors is greater than capacity, make a reallocation
		if (graph->edges[_source].size >= graph->edges[_source].capacity)
		{	
			graph->edges[_source].capacity *= 2;
			reallocation(&graph->edges[_source].adjacents,graph->edges[_source].capacity,_fd);
		}

		// Add it into the graph
		graph->edges[_source].vertexID = _source;
		addEdge(graph,_edge);

		// Go to the bottom line
		token = strtok_r(_buf,"\n",&_buf);
	}

	free(temp);
}


int isPresent(struct cacheList* _cache, int* _size, int _source, int _dest)
{
	// If requested destination and source were calculated before, it returns one
	// Otherwise, it returns zero 
	for (int i = 0; i < *_size; ++i)
	{
		if (_cache[i].edge.source == _source && _cache[i].edge.dest == _dest) {
			return 1;
		}
	}

	return 0;
}


int checkPastCalculations(struct cacheList* _cache, int* _size, char* path, int _source, int _dest)
{
	// If requested destination and source were calculated before, copy it into path array and return one
	// Otherwise, return zero 
	for (int i = 0; i < *_size; ++i)
	{
		if (_cache[i].edge.source == _source && _cache[i].edge.dest == _dest) {
			strcpy(path,_cache[i].graphPath);
			return 1;
		}
	}

	return 0;
}

int findPath_usingBFS(int* path, int* _count, int _source, int _dest, struct Graph* _graph, int _fd)
{
	// Hold an array to construct bfs tree
	int* parent = (int*) malloc(sizeof(int)*_graph->numOfVertices);
	if (parent == NULL)
	{
		write(_fd,"Cannot allocate memory for parent array in findPath_usingBFS function\n",71);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	for (int i = 0; i < _graph->numOfVertices; ++i)
	{
		parent[i] = -1;
	}

	int k = 0, neighbor, cur;

	struct Queue* _queue = (struct Queue*) malloc(sizeof(struct Queue)*1);
	if (_queue == NULL)
	{
		write(_fd,"Cannot allocate memory for queue in findPath_usingBFS function\n",64);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	createQueue(_queue,_graph->numOfVertices,_fd);
	
	// Array to hold nodes that is visited before
	int* visited = (int*) malloc(sizeof(int)*_graph->numOfVertices);
	if (visited == NULL)
	{
		write(_fd,"Cannot allocate memory for visited array in findPath_usingBFS function\n",72);
		if (shm_unlink("prevent_two_instantiation") == -1)
		{
			write(_fd,"Failed to unlink shared memory\n",32);
			exit(1);
		}
		exit(1);
	}

	for (int i = 0; i < _graph->numOfVertices; ++i)
	{
		visited[i] = 0;
	}

	// Add the starting vertex into the queue and mark the visited array
	visited[_source] = 1;
	addElement(_queue,_source);

	// Continue until queue is empty
	while(isEmpty(_queue) != 1) {
		
		// Take the first element and remove it from queue
		cur = removeFront(_queue);
		// If the current element equals to destination vertex, path is found
		if (cur == _dest)
		{	
			path[k] = _dest;
			++k;
			int num = _dest;
			// fill the path array according to the bfs tree
			while (num != _source) {

				path[k] = parent[num];
				num = path[k];
				++k;
			}

			int beg = 0, end = k - 1;
			int tmp;
			// Reverse the path array
			while(beg < end) {

				tmp = path[beg];
				path[beg] = path[end];
				path[end] = tmp;
				++beg;
				--end;
			}

			// _count indicates fullness of the path array
			*_count = k;
			free(parent);
			free(visited);
			free(_queue->array);
			free(_queue);
			return 1;
		}

		// Otherwise, find out all neighbors of the current 
		for (int i = 0; i < _graph->edges[cur].size; ++i)
		{
			neighbor = _graph->edges[cur].adjacents[i].dest;
			// If the neighbor vertex is not visited, mark the visited array and add the element 
			if (visited[neighbor] != 1)
			{
				visited[neighbor] = 1;
				addElement(_queue,neighbor);
				parent[neighbor] = cur;
			}
		}
	}

	free(parent);
	free(visited);
	free(_queue->array);
	free(_queue);
	return 0;
}


/*   Print Messages About the State of the Client Process   */ 
void client_printConnecting(int _pid, char* _ip, int* _port, char* curTime)
{
	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Client (%d) connecting to %s:%d\n",curTime,_pid,_ip,*_port);
	write(STDOUT_FILENO,text,strlen(text));
}

void client_Connected(int _pid, int* _source, int* _dest, char* curTime)
{
	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Client (%d) connected and requesting a path from node %d to %d\n",curTime,_pid,*_source,*_dest);
	write(STDOUT_FILENO,text,strlen(text));
}

void client_getAnswer(int _pid, char* _ans, double _time, char* curTime)
{
	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[100000];
	sprintf(text,"%s Server’s response to (%d): %s, arrived in %.5fseconds.\n",curTime,_pid,_ans,_time);
	write(STDOUT_FILENO,text,strlen(text));
}


/*   Print Messages About the State of the Client Process   */ 
void server_Executing(int _fd, char* input, char* output, int* _port, int* _s, int* _x)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[500];
	sprintf(text,"%s Executing with parameters:\n-i %s\n-p %d\n-o %s\n-s %d\n-x %d\n",curTime,input,*_port,output,*_s,*_x);
	write(_fd,text,strlen(text));
}

void server_loadingGraph(int _fd)
{
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[200];
	sprintf(text,"%s Loading graph...\n",curTime);
	write(_fd,text,strlen(text));
}

void server_graphLoaded(int _fd, double elapsed_time, int _vertex, int _edges)
{
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Graph loaded in %.5f seconds with %d nodes and %d edges.\n",curTime,elapsed_time,_vertex,_edges);
	write(_fd,text,strlen(text));
}

void server_threadCreated(int _fd, int* _s)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s A pool of %d threads has been created\n",curTime,*_s);
	write(_fd,text,strlen(text));
}

void server_threadWaitingConnection(int _fd, int _index)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Thread #%d: waiting for connection\n",curTime, _index);
	write(_fd,text,strlen(text));
}

void server_connectionDelegated(int _fd, int _index, double systemLoad)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s A connection has been delegated to thread id #%d, system load %.3f\n",curTime, _index, systemLoad);
	write(_fd,text,strlen(text));
}

void server_threadSearchingDatabase(int _fd, int _index, int _source, int _dest)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Thread #%d: searching database for a path from node %d to node %d\n",curTime, _index, _source,_dest);
	write(_fd,text,strlen(text));
}

void server_noPathInDatabase(int _fd, int _index, int _source, int _dest)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Thread #%d: no path in database, calculating %d->%d\n",curTime, _index, _source,_dest);
	write(_fd,text,strlen(text));
}

void server_pathCalculated(int _fd, int _index, char* _ans)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[100000];
	sprintf(text,"%s Thread #%d: path calculated: %s\n",curTime, _index, _ans);
	write(_fd,text,strlen(text));
}

void server_addingPathToCache(int _fd, int _index)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Thread #%d: responding to client and adding path to database\n",curTime, _index);
	write(_fd,text,strlen(text));
}

void server_pathNotPossible(int _fd, int _index, int _source, int _dest)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Thread #%d: path not possible from node %d to %d\n",curTime, _index, _source, _dest);
	write(_fd,text,strlen(text));
}

void server_pathFoundInCache(int _fd, int _index, char* _ans)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Thread #%d: path found in database: %s\n",curTime, _index, _ans);
	write(_fd,text,strlen(text));
}

void server_poolExtended(int _fd, int* _num)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s System load 75%%, pool extended to %d threads\n",curTime, *_num);
	write(_fd,text,strlen(text));
}

void server_noThreadAvailable(int _fd)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s No thread is available! Waiting for one.\n",curTime);
	write(_fd,text,strlen(text));
}

void server_signalReceived(int _fd)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s Termination signal received, waiting for ongoing threads to complete.\n",curTime);
	write(_fd,text,strlen(text));
}

void server_threadsTerminated(int _fd)
{	
	time_t _time = time(NULL);
	char* curTime = ctime(&_time);

	int last = strlen(curTime);
	// Remove the new line from string
	if (curTime[last-1] == '\n')
		curTime[last-1] = '\0';

	char text[300];
	sprintf(text,"%s All threads have terminated, server shutting down.\n",curTime);
	write(_fd,text,strlen(text));
}

