#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include "calculations.h"


int output_fd;
char input[20];

// to understand if the p1 process is finished.
int isFinished = 0;


// SIGUSR1 handler
// If SIGUSR1 signal is sent, that means p1 process finished.
void handler(int sig_no)
{
	isFinished = 1;
	unlink(input);
}

// SIGUSR2 handler
void handler2(int sig_no)
{
	/* EMPTY */
}

// SIGTERM handler
void handler3(int sig_no)
{

	if (-1 == close(output_fd))
	{
		perror("Error occured");
		exit(1);
	}
}


void readLinebyLine(char* temp_name, int _f)
{	
	output_fd = _f;
	struct flock lock;
	memset(&lock,0,sizeof(lock));

	int wrtOffset = 0, tmp_count = 0, tmp_wr;
	int count = 0, totalLine = 0;
	int fd, size = -1 ,i, k, j, tour, m = 0;

	double mae, mse, rmse;
	double a,b;

	char ch = ' ';

	struct coordinate c[10];
	char c_buf[15], lineEquation[20], MAE[20], MSE[20], RMSE[20];

	sigset_t set, pendingSet, newMask;

	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigaction(SIGINT,&sa,NULL);


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


	// If SIGUSR2 signal arrives, this execution can continue, otherwise it will wait.
	// If it waits, that means p1 does not processed any line.
	sigfillset(&newMask);
	sigdelset(&newMask,SIGUSR2);
	sigsuspend(&newMask);

	// Open the temporary file in read and write mode
	fd = open(temp_name,O_RDWR);		
	if (fd == -1)
	{
		perror("Failed to open temporary file");
		exit(1);
	}
		
	// Calculate the characters of the first row.
	while(ch != '\n'){

		size = read(fd,&ch,1);
		++count;
		if (size == -1)
		{
			perror("Error is occured : \n");
			exit(1);
		}

		// Total line is needed for while calculating mean and standar deviation of error metrics.
		// totalLine variable represents how many calculations is made.
		if (ch == 'x')
		{
			totalLine+=1;
		}
	}

	double* errors = (double*) malloc(sizeof(double)*1000000);;
	char* num = (char*) malloc(sizeof(char)*10);
	char* buf = (char*) malloc(sizeof(char)*count+1);
	char* spaceBuf = (char*) malloc(sizeof(char)*count+1);

	// For erasing lines.
	for (i = 0; i < count; ++i)
	{
		spaceBuf[i] = ' ';
	}
	spaceBuf[count] = '\0';

	// Set the offset
	lseek(fd,0,SEEK_SET);

	size = -1;
	while(!(size == 0 && isFinished == 1)){

		// read the whole row
		size = read(fd,buf,count);
		if (size == -1)
		{
			perror("Failed to read temporary file");
			exit(1);
		}

		j = 0;
		tour = 0;
		
			for (i = 0; i < count; ++i)
			{
				if (buf[i] != '(' && buf[i] != ')' && buf[i] != ',' && buf[i] != ' ')
				{
					k = 0;
					// Parse the string
					while(buf[i] != ',' && buf[i] != ')')
					{	
						// Save the coordinates in num as string form 
						num[k] = buf[i];
						++k;
						++i;
					}

					// After that save numbers in coordinate array
					if (tour % 2 == 0)
					{	
						c[j].x = atoi(num);
					}

					else{
						c[j].y = atoi(num);
						++j;
					}

					free(num);
					num = (char*) malloc(sizeof(char)*10);

					++tour;	
				}

				// If j equals to ten, it means whole row is processed, now line equation can be processed.
				if (j == 10)
				{	
					k = 0;
					while(buf[i] != 'x')
					{	
						if (buf[i] != '(' && buf[i] != ')' && buf[i] != ',' && buf[i] != ' ')
						{
							num[k] = buf[i];
							++k;
						}

						++i;
					}

					a = atof(num);
					free(num);
					num = (char*) malloc(sizeof(char)*10);
					++i; // to get rid of the x byte

					k = 0;
					while(buf[i] != '\n'){

						if (buf[i] != '+' && buf[i] != ',' && buf[i] != ' ')
						{
							num[k] = buf[i];
							++k;
						}

						++i;
					}

					b = atof(num);

					i = count;

					free(num);
					num = (char*) malloc(sizeof(char)*10);

				}
			}	

		tmp_count = tmp_count + count;
		tmp_wr = count;
		count = 0;
		ch = ' ';

		// Calculate total bytes of the next row 
		while(ch != '\n'){

			size = read(fd,&ch,1);
			if (size == -1)
			{
				perror("Failed to read temporary file");
				exit(1);
			}

			// Calculate how many lines processed
			if (ch == 'x')
			{
				totalLine+=1;
			}

			// If size equals to zero, it means there are no more bytes to be processed
			// and there can be two reasons of that.
			// p1 has not finished yet or p1 processed all the contents, p2 reached the EOF 
			if (size == 0)
			{	
				ch = '\n';
				if (isFinished != 1)
				{	
					// Wait for p1, so it can process more lines.
					sigsuspend(&newMask);
					ch = ' ';
					--count;
				}
			}

			++count;		
		}

		if (size != 0)
		{	
			free(buf);
			buf = (char*) malloc(sizeof(char)*count+1);
		}

		/* CRITICAL SECTION */
		if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
		{
			perror("Error is occured : \n");
		}

		calculateMAE(&mae,c,a,b);
		calculateMSE(&mse,c,a,b);
		calculateRMSE(&rmse,mse);
		if (sigpending(&pendingSet) == -1)
		{
			perror("Error is occured : \n");
			exit(1);
		}

		if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
		{
			perror("Error is occured : \n");
			exit(1);
		}
		/* CRITICAL SECTION IS OVER */

		// Lock the file in case other process try to write to it.
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW,&lock);

		// Set the offset
		lseek(fd,wrtOffset,SEEK_SET);
		if (-1 == write(fd,spaceBuf,strlen(spaceBuf)))
		{
			perror("Failed to erase the line");
			exit(1);
		}

		lock.l_type = F_UNLCK;
		fcntl(fd,F_SETLKW,&lock);	

		for (i = 0; i < 10; ++i)
		{
			sprintf(c_buf,"(%d, %d) , ", c[i].x, c[i].y);
			if (-1 == write(_f,c_buf,strlen(c_buf)) )
			{	
				perror("Failed to write to output file");
				exit(1);
			}
		}

		// Convert error metrics and line equation to string
		sprintf(lineEquation,"%0.3fx + %0.3f, ",a,b);
		sprintf(MAE,"%0.3f, ",mae);
		sprintf(MSE,"%0.3f, ",mse);
		sprintf(RMSE,"%0.3f\n",rmse);

		// Save it in the array
		errors[m] = mae;
		errors[m+1] = mse;
		errors[m+2] = rmse;
		m = m + 3;


		if (-1 == write(_f,lineEquation,strlen(lineEquation)) )
		{
			perror("Failed to write to output file");
			exit(1);
		}

		if (-1 == write(_f,MAE,strlen(MAE)) )
		{
			perror("Failed to write to output file");
			exit(1);
		}

		if (-1 == write(_f,MSE,strlen(MSE)) )
		{
			perror("Failed to write to output file");
			exit(1);
		}

		if (-1 == write(_f,RMSE,strlen(RMSE)) )
		{
			perror("Failed to write to output file");
			exit(1);
		}
		
		// Update the offset
		wrtOffset =wrtOffset + tmp_wr;

		free(spaceBuf);
		spaceBuf = (char*) malloc(sizeof(char)*count+1);;
		for (i = 0; i < count; ++i)
		{
			spaceBuf[i] = ' ';
		}
		spaceBuf[count] = '\0';

		// Update the offset for reading operation
		lseek(fd,tmp_count,SEEK_SET);
		
	}

	double MAEmean = 0;
	double MSEmean = 0;
	double RMSEmean = 0;
	double MAEdeviation = 0;
	double MSEdeviation = 0;
	double RMSEdeviation = 0;

	for (int i = 0; i < m; i+=3)
	{
		MAEmean += errors[i];
		MSEmean += errors[i+1];
		RMSEmean += errors[i+2];
	}

	MAEmean = MAEmean/totalLine;
	MSEmean = MSEmean/totalLine;
	RMSEmean = RMSEmean/totalLine;

	double r1 = 0;
	double r2 = 0;
	double r3 = 0;
	for (int i = 0; i < m; i+=3)
	{
		r1 = pow((errors[i] - MAEmean),2);
		MAEdeviation = MAEdeviation + r1;

		r2 = pow((errors[i+1] - MSEmean),2);
		MSEdeviation = MSEdeviation + r2;

		r3 = pow((errors[i+2] - RMSEmean),2);
		RMSEdeviation = RMSEdeviation + r3;	
	}

	MAEdeviation = MAEdeviation / (totalLine -1);
	MSEdeviation = MSEdeviation / (totalLine -1);
	RMSEdeviation = RMSEdeviation / (totalLine -1);

	MAEdeviation = sqrt(MAEdeviation);
	MSEdeviation = sqrt(MSEdeviation);
	RMSEdeviation = sqrt(RMSEdeviation);

	// When p2 is over, Print on the screen mean and the standard deviation of each error metric.
	printf("MAE MEAN : %0.3f, MSE MEAN : %0.3f, RMSE MEAN : %0.3f\n",MAEmean,MSEmean,RMSEmean );
	printf("MAE SD : %0.3f, MSE SD : %0.3f, RMSE SD : %0.3f\n",MAEdeviation ,MSEdeviation ,RMSEdeviation);

	if (close(fd) == -1 || close(_f) == -1)
	{
		perror("Failed to close open files");
		exit(1);
	}


	free(errors);
	free(spaceBuf);
	free(num);
	free(buf);
}


int main(int argc, char *argv[])
{	
	strcpy(input,argv[3]);
	sigset_t _set;
	// Create a set and add it to SIGUSR2 signal.
	// This signal is necessary for sigsuspend situation.
	if (sigemptyset(&_set) == -1 || sigaddset(&_set,SIGUSR2) == -1)	
	{
		perror("Failed to initialize signal mask");
		exit(1);
	}
	// SIGUSR2 signal should not be sent before the sigsuspend line is executed. 
	// That's way I blocked it. 
	if (sigprocmask(SIG_BLOCK,&_set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		exit(1);
	}

	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &handler;
	
	struct sigaction sa2;
	memset(&sa2,0,sizeof(sa2));
	sa2.sa_handler = &handler2;
	
	struct sigaction sa3;
	memset(&sa3,0,sizeof(sa3));
	sa3.sa_handler = &handler3;

	sigaction(SIGUSR1,&sa,NULL);
	sigaction(SIGUSR2,&sa2,NULL);
	sigaction(SIGTERM,&sa3,NULL);

	// Send SIGUSR1 signal to parent process. It indicates the child process has started.
	if(-1 == kill(getppid(),SIGUSR1))
	{
		perror("Failed to send a signal to parent process");
		exit(1);
	}
	
	readLinebyLine(argv[1],atoi(argv[2]));

	return 0;
}