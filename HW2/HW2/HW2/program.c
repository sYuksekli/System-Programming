#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "calculations.h"

// to understand if the p2 process is started.
int isStarted = 0;

int isOpenedInput = 0;
int isOpenedTemp = 0;
char Tempname[14];
char inputname[20];

// Handler for SIGUSR1 signal.
// When p2 is started, it will send this signal to p1 process to notify
void sigHandler(int sig)
{
	isStarted = 1;
}

void sigHandler2(int sig)
{
	if (isOpenedTemp == 1)
	{
		unlink(Tempname);
	}
	
	if (isOpenedInput == 1)
	{
		unlink(inputname);
	}
}


/*
** Read the input file and whenever it reads 20 bytes it writes them to the temporary file.
*/
pid_t readFromFile(char* readFile, char* writeFile)
{
	strcpy(inputname,readFile);

	int fd, descriptor, size = 20;
	int i, k, count = 0;
	double a,b;
	struct coordinate c[10];
	char lineEquation[20], c_buf[15];
	char name[] = "templateXXXXXX";
	char* buf = (char*) malloc(sizeof(char)*20);

	// Open the temporary file
	descriptor = mkstemp(name);
	if (descriptor == -1)
	{	
		perror("Failed to open temporary file");
		exit(1);
	}

	// In case of sigterm
	isOpenedTemp = 1;
	strcpy(Tempname,name);

	// Create the output file in write only mode
	int des;
	des = open(writeFile,O_CREAT | O_WRONLY);	
	if (des == -1)
	{
		perror("Failed to create outputPath file");
		exit(1);
	}

	char _des[2];
	sprintf(_des,"%d",des);
	
	char* const list[] = {"program2",name,_des,readFile,NULL};
	pid_t id;


	// Open the input file in readonly mode
	fd = open(readFile,O_RDONLY);		
	if (fd == -1)
	{
		perror("Failed to open input file");
		exit(1);
	}
	
	// Create child process and executes it.
	// Give to it arguments that inclueds temporary file name, outputPath file descriptor and inputPath file name.
	fflush(stdout);
	id = fork();
	if (id == 0)
	{	
		execve("program2",list,NULL);
	}

	struct flock lock;
	memset(&lock,0,sizeof(lock));

	sigset_t set, pendingSet;

	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = SIG_IGN;

	if (sigemptyset(&set) == -1 || sigaddset(&set,SIGINT) == -1)
	{
		perror("Failed to initialize signal mask");
		exit(1);
	}

	if (sigemptyset(&pendingSet) == -1)
	{
		perror("Failed to initialize pending mask");
		exit(1);
	}

	sigaction(SIGINT,&sa,NULL);


	// In case of SIGTERM
	isOpenedInput = 1;

	// Continue if it reads 20 bytes
	while (size == 20)					
	{
		size = read(fd,buf,20);
		if (size == -1)
		{
			perror("Failed to read input file");
			exit(1);
		}

		k = 0;
		if (size == 20)	
		{	
			++count;	// To calculate how many bytes it has read
			for (i = 0; i < 20; ++i)
			{	
				// Save these numbers into the coordinate array
				if (i % 2 == 0 )
				{
					c[k].x = (unsigned char) buf[i];
				}

				else{
					c[k].y = (unsigned char) buf[i];
					++k;
				}
			}

			// It is imposibble to caught or ignore SIGSTOP signal.
			/* CRITICAL SECTION */
			if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
			{
				perror("Blocking operation is failed");
				exit(1);
			}

			leastSquareMethod(&a, &b, c);
			if (sigpending(&pendingSet) == -1)	
			{
				perror("Error is occured");
				exit(1);
			}

			if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
			{
				perror("Failed to unblock operation");
				exit(1);
			}
			/* CRITICAL SECTION IS OVER */


			// Create a string that represents line equation
			sprintf(lineEquation,"%0.3fx + %0.3f\n",a,b);

			// Lock the file in case another process try to write.
			lock.l_type = F_WRLCK;
			fcntl(descriptor,F_SETLKW,&lock);
			for (i = 0; i < 10; ++i)
			{
				sprintf(c_buf,"(%d, %d) , ", c[i].x, c[i].y);
				if (-1 == write(descriptor,c_buf,strlen(c_buf)) )
				{	
					perror("Failed to write to temporary file");
					exit(1);
				}
			}

			if (-1 == write(descriptor,lineEquation,strlen(lineEquation)) )
			{
				perror("Failed to write to temporary file");
				exit(1);
			}

			lock.l_type = F_UNLCK;
			fcntl(descriptor,F_SETLKW,&lock);
		}	

		// If p2 has started, send SIGUSR2 signal to it, because the operation should be performed as soon as possible.
		// If context switch happens before the p1 ends, by this way p2 can perform operation on the line which
		// is written  by p1.
		if (isStarted == 1)
		{
			kill(id, SIGUSR2);
		}
		
	}

	// After p1 processed all the contents of the input file, it closes the open files.
	if (close(fd) == -1 || close(descriptor) == -1)
	{
		perror("Failed to close open files");
		exit(1);
	}

	// Print informations about how many bytes have written and line equations are estimated on the screen.
	printf("P1 process read %d bytes and estimated %d line equations. \n", count*20, count);
	// If any signal is sent while p1 was in the critical section print on the screen.
	if (sigismember(&pendingSet,SIGINT))
	{
		printf("SIGINT signal were sent to P1 while it was in the critical section \n");
	}


	free(buf);
	return id;
}




int main(int argc, char *argv[])
{	
	pid_t pid;
	int stat;

	struct sigaction sAct;
	memset(&sAct,0,sizeof(sAct));
	sAct.sa_handler = &sigHandler;
	sigaction(SIGUSR1,&sAct,NULL);

	struct sigaction sAct2;
	memset(&sAct2,0,sizeof(sAct2));
	sAct2.sa_handler = &sigHandler2;
	sigaction(SIGTERM,&sAct2,NULL);

	// Save command line arguments into this variables.
	char* Rfilename = (char*) malloc(sizeof(char)*100);
	char* Wfilename = (char*) malloc(sizeof(char)*100);

	// Parse command line
	int count = 0; // To check all the options are entered by the user.
	int opt;
	while((opt = getopt(argc, argv, ":i:o:")) != -1)
	{
		switch(opt)
		{
			case 'i':
			{	
				++count;
				strcpy(Rfilename,optarg);
				break;
			}

			case 'o':
			{
				++count;
				strcpy(Wfilename,optarg);
				break;
			}

			case ':':
			{
				printf("option needs a value\n");
				break;
			}

			case '?':
			{
				printf("Unknown option %c,You must enter a command line like this : -i input -o output\n", optopt);
				exit(1);
				break;
			}
		}
	}

	for ( ; optind < argc; ++optind)
	{
		printf("You entered extra arguments %s,You must enter a command line like this : -i input -o output\n",argv[optind]);
		exit(1);
	}

	if (count == 2){
		// Return value waitpid is needed for waitpid.
		pid = readFromFile(Rfilename,Wfilename);
	}

	else{
		printf("You entered missing command lines, You must enter a command line like this : -i input -o output\n");
		exit(1);
	}


	while(isStarted != 1)
	{
		
	}

	// If p2 process is started, send it to SIGUSR1 signal to notify that p1 process is finished.
	// If p2 process is started, send it to SIGUSR2 signal to prevent it to hanging in sigsuspend line. 
	if (isStarted == 1)
	{	
		if(-1 == kill(pid,SIGUSR1))
		{
			perror("Failed to send a signal to child process");
			exit(1);
		}
		
		
		if(-1 == kill(pid,SIGUSR2))
		{
			perror("Failed to send a signal to chilf process");
			exit(1);
		}
	}


	waitpid(pid,&stat,0);

	//Free all resources.
	free(Rfilename);
	free(Wfilename);
	return 0;

}