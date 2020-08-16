#include "helper.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>


// It holds the addresses of pointers to use in free operation
void** addrs;
// If addrs pointer is allocated, i change this variable 
int allocated = 0;
// size of the addrs pointer
int count = 0;
// File descriptor of the socket
int client_fd;
// to understand if the socket fd is opened
int opened = 0;
// set for sigint signal
sigset_t set;


// Free resources
void exitGracefully();
// To create a critical section, blocking and unblocking operations
void begOfCriticalSection();
void endOfCriticalSection();
// SIGINT handler
void handler(int signo);
// To parse command line arguments
void parseArgs(int _argc, char* _argv[], char* _addr, int* _port, int* _s, int* _d);


int main(int argc, char *argv[])
{


	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &handler;

	// To catch SIGINT
	if (sigaction(SIGINT,&sa,NULL) == -1)
	{
		perror("sigaction error is occured");
		exit(1);
	}

	// I block SIGINT in some cases like allocating resources to prevent free errors
	if (sigemptyset(&set) == -1 || sigaddset(&set,SIGINT) == -1)
	{
		perror("Failed to initialize signal mask");
		exit(1);
	}

	// Get the pid of the process
	int client_pid = getpid();


	begOfCriticalSection();
	addrs = (void**) malloc(sizeof(void*)*1000);
	allocated = 1;
	endOfCriticalSection();

	// Command line arguments, I allocate space for them
	begOfCriticalSection();
	char* ip = (char*) malloc(sizeof(char)*100);
	addrs[count] = ip;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	int* s = (int*) malloc(sizeof(int)*1);
	addrs[count] = s;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	int* d = (int*) malloc(sizeof(int)*1);
	addrs[count] = d;
	++count;
	endOfCriticalSection();

	begOfCriticalSection();
	int* port = (int*) malloc(sizeof(int)*1);
	addrs[count] = port;
	++count;
	endOfCriticalSection();

	// Parse command line arguments
	parseArgs(argc, argv, ip, port, s, d);

	// To measure elapsed time
	struct timeval t1;
	struct timeval t2;

	// Create socket
	client_fd = socket(AF_INET,SOCK_STREAM,0);
	if (client_fd == -1)
	{
		perror("Failed to create socket");
		exitGracefully();
	}
	begOfCriticalSection();
	opened = 1;
	endOfCriticalSection();


	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(*port);
	// Convert address sting to binary form
	if (inet_pton(AF_INET,ip,&addr.sin_addr) <= 0)
	{
		perror("Invalid address");
		exitGracefully();
	}


	// to print the current timestamp
	begOfCriticalSection();
	time_t _time = time(NULL);
	char* timeString = ctime(&_time);
	endOfCriticalSection();

	// print that the client is connecting to server
	client_printConnecting(client_pid, ip, port, timeString);

	// Connect to the server
	if (connect(client_fd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		perror("Failed to connect");
		exitGracefully();
	}

	// Alocate space for requested path
	begOfCriticalSection();
	char* request = (char*) malloc(sizeof(char*)*1000);
	addrs[count] = request;
	++count;
	endOfCriticalSection();

	// Convert the source and destination value to string and save it in request string
	sprintf(request,"%d - %d\n",*s,*d);
	int result = write(client_fd,request,1000);
	gettimeofday(&t1,0);	// to measure the time between request and answer 
	if (result == -1)
	{
		perror("Failed to write to socket");
		exitGracefully();
	}

	// print the current timestamp
	begOfCriticalSection();
	time_t _time2 = time(NULL);
	char* timeString2 = ctime(&_time2);
	endOfCriticalSection();
	// print the request
	client_Connected(client_pid, s, d, timeString2);

	// Allocate space for answer string
	begOfCriticalSection();
	char* answer = (char*) malloc(sizeof(char*)*10000);
	addrs[count] = answer;
	++count;
	endOfCriticalSection();

	// Read from string and save the answer into the answer array
	result = read(client_fd,answer,10000);
	gettimeofday(&t2,0); 		// answer of request is taken
	if (result == -1)
	{
		perror("Failed to read from socket");
		close(client_fd);
		exitGracefully();
	}

	// If answer is path not possible, change is to no path 
	if (answer[0] == 'p')
	{
		strcpy(answer,"NO PATH");
	}

	// Calculate total time spending while waiting for an answer
	double elapsedTime = t2.tv_sec + t2.tv_usec / 1e6 - t1.tv_sec - t1.tv_usec / 1e6;

	begOfCriticalSection();
	// print the current timestamp
	time_t _time3 = time(NULL);
	char* timeString3 = ctime(&_time3);
	endOfCriticalSection();

	// print the answer
	client_getAnswer(client_pid, answer, elapsedTime, timeString3);



	// Free resources
	begOfCriticalSection();
	for (int i = count -1; i >= 0; --i){
		free(addrs[i]);
		--count;
	}

	if (allocated == 1)
		free(addrs);

	close(client_fd);
	endOfCriticalSection();


	return 0;
}


// To parse command line aguments, I also give the addrs as parameter to exit gracefully in case of an error
void parseArgs(int _argc, char* _argv[], char* _addr, int* _port, int* _s, int* _d)
{
	int opt;
	int count = 0; // To check all the options are entered by the user.
	while((opt = getopt(_argc, _argv, ":a:p:s:d:")) != -1)
	{
		switch(opt)
		{

			case 'a':
			{
				strcpy(_addr,optarg);
				++count;
				break;
			}

			case 'p':
			{
				*_port = atoi(optarg);
				++count;
				break;
			}
			case 's':
			{

				*_s = atoi(optarg);
				if (*_s < 0)
				{
					write(STDOUT_FILENO,"Index of nodes must be greater than zero\n",42);
					exitGracefully();
				}

				for (int i = 0; i < strlen(optarg); ++i)
				{
					if (optarg[i] < 48 || optarg[i] > 57)
					{
						write(STDOUT_FILENO,"Please enter integer\n",22);
						exitGracefully();
					}
				}

				++count;
				break;
			}
			case 'd':
			{
	
				*_d = atoi(optarg);
				if (*_d < 0)
				{
					write(STDOUT_FILENO,"Index of nodes must be greater than zero\n",42);
					exitGracefully();
				}

				for (int i = 0; i < strlen(optarg); ++i)
				{
					if (optarg[i] < 48 || optarg[i] > 57)
					{
						write(STDOUT_FILENO,"Please enter integer\n",22);
						exitGracefully();
					}
				}

				++count;
				break;
			}
			

			case ':':
			{
				write(STDOUT_FILENO,"option needs a value\n",22);
				exitGracefully();
			}

			case '?':
			{
				char text[200];
				sprintf(text,"Unknown option %c,You must enter a command line like this : -a 127.0.0.1 -p PORT -s 768 -d 979\n",optopt);
				write(STDOUT_FILENO,text,strlen(text));
				exitGracefully();
			}
		}
	}

	for ( ; optind < _argc; ++optind)
	{
		char text[300];
		sprintf(text,"You entered extra arguments %s,You must enter a command line like this : -a 127.0.0.1 -p PORT -s 768 -d 979\n",_argv[optind]);
		write(STDOUT_FILENO,text,strlen(text));
		exitGracefully();
	}

	if (count != 4)
	{
		write(STDOUT_FILENO,"You entered missing command lines, You must enter a command line like this : -a 127.0.0.1 -p PORT -s 768 -d 979\n",113);
		exitGracefully();
	}
}

// To block sigint signal to exit gracefully 
void begOfCriticalSection()
{
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		exitGracefully();
	}
}

// To unblock sigint signal
void endOfCriticalSection()
{
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Unblocking operation is failed");
		exitGracefully();
	}
}

// signal handler
void handler(int signo)
{
	exitGracefully();
}

// Free resources
void exitGracefully()
{	
	begOfCriticalSection();
	for (int i = count -1; i >= 0; --i){
		free(addrs[i]);
		--count;
	}

	if (allocated == 1)
		free(addrs);

	if (opened == 1)
		close(client_fd);
	
	endOfCriticalSection();
	exit(1);
}
