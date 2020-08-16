#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>
#include "helper.h"




void** param = NULL;	
struct florist* florists = NULL;
struct Queue* requests = NULL;
pthread_mutex_t* mutexes = NULL;
pthread_cond_t* full = NULL;
pthread_cond_t* isFinished = NULL;
struct Statistic** Tmp = NULL;
int* Count = NULL;
int* flag = NULL;
int* order = NULL;
char* filename = NULL;
char* buf = NULL;
int threadCount = 0; 
int clientCount = 0;
int isOpened = 0;
int fd;



// Handler for SIGINT signal.
void  handler(int sig)
{
	if (filename != NULL)
		free(filename);
	if (buf != NULL)
		free(filename);
	if (Count != NULL)
		free(filename);
	if (flag != NULL)
		free(filename);
	if (order != NULL)
		free(filename);
	if (Tmp != NULL)
	{
		for (int i = 0; i < threadCount; ++i)
		{
			free(Tmp[i]);
		}
		free(Tmp);
	}

	if (florists != NULL)
	{
		for (int i = 0; i < threadCount; ++i)
		{	
			free(florists[i].name);
		}

		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);
	}

	if (requests != NULL)
	{
		for (int k = 0; k < threadCount; ++k)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[k].request[j]);
			}
			free(requests[k].request);
		}
		
		for (int j = 0; j < threadCount; ++j)
		{
			free(requests[j].distance);
		}

		for (int k = 0; k < threadCount; ++k)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[k].client_no[j]);
			}

			free(requests[k].client_no);
		}

		free(requests);
	}

	if (mutexes != NULL)
	{
		for (int i = 0; i < threadCount; ++i)
		{
			pthread_mutex_destroy(&mutexes[i]);
		}
		free(mutexes);
	}
	
	if (full != NULL)
	{
		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&full[j]);
		}
		free(full);		
	}
	
	if (isFinished != NULL)
	{
		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&isFinished[j]);
		}
		free(isFinished);
	}
	

	if (param != NULL)
	{
		for (int i = 0; i < threadCount; ++i)
		{
			free(param[i]);
		}
		free(param);
	}

	if (isOpened == 1)
	{
		close(fd);
	}

	exit(1);
}



// Cook function
void* floristFunction(void* parameters)
{	

	srand(time(NULL));
	// Which florist is this
	int index = *((int*) parameters); 
	float dis, _speed;
	float timeOfDelivery, timeOfPreparation, deliveredTime, totalTime = 0;

	while(1) {


		// Lock
		pthread_mutex_lock(&mutexes[index]);
		while(Count[index] == 0) {
			pthread_cond_wait(&full[index],&mutexes[index]);
		} 
		if (flag[index] == 1)
		{
			--Count[index];
		}
		if (flag[index] == 1 && Count[index] == 0)
		{
			pthread_mutex_unlock(&mutexes[index]);
			break;
		}

		timeOfPreparation = rand() % 250 + 1; 			// between 0 and 251 msec
		dis = requests[index].distance[order[index]];	// distance between proper florist and client
		_speed = florists[index].speed;					// speed of florist
		timeOfDelivery = dis / _speed;					// x = v*t

		deliveredTime = timeOfDelivery + timeOfPreparation;
		totalTime = totalTime + deliveredTime;
		usleep(deliveredTime*1000);						// It takes microsec, multiple it by 1000
		// Print on the screen
		florist_hasDelivered(florists[index].name,requests[index].request[order[index]],requests[index].client_no[order[index]],deliveredTime);
	
		// Consume item
		Count[index] -= 1;   
		// Next item 
		order[index] += 1;
		// Send a signal every time so that central thread can control if the florist is done
		pthread_cond_signal(&isFinished[index]);
		// Unlock
		pthread_mutex_unlock(&mutexes[index]);

	}

	// Set statissic
	struct Statistic* statistics = (struct Statistic*) malloc(sizeof(struct Statistic)*1);
	statistics->total_time = totalTime;
	statistics->totalOrder = order[index];

	// prints that it is done
	florist_Closing(florists[index].name);
	// return statistic
	return (void*) statistics;
}




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

	// I block SIGINT in some cases like freeing resources
	// and also, I postpone it if threads is started already
	sigset_t set;
	if (sigemptyset(&set) == -1 || sigaddset(&set,SIGINT) == -1)
	{
		perror("Failed to initialize signal mask");
		exit(1);
	}



	// count is the number of command line arguments
	int count;
	filename = (char*) malloc(sizeof(char)*1000);
	count = parseCommandLine(argc, argv, filename);
	if (count != 1)
	{
		write(STDOUT_FILENO,"You entered missing command lines, You must enter a command line like this : -i filePath\n",90);
		free(filename);
		exit(1);
	}




	// critical section
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(filename);
		exit(1);
	}
	// Open the input file in read only mode
	fd = open(filename,O_RDONLY);		
	if (fd == -1)
	{
		perror("Failed to open input file");
		free(filename);
		exit(1);
	}

	// In case of CTRL-C, if file is opened, it should be closed. So I created a critical section and changed the 
	// value of isOpened variable so that I can understand whether it is opened or not.
	isOpened = 1;
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(filename);
		close(fd);
		exit(1);
	}
	// End of critical section


	int size;
	int  buf_size = lseek(fd,0,SEEK_END);					// get the size of the file
	buf = (char*) malloc(sizeof(char)*(buf_size+1));		// Read file into an array

	// Read the file
	lseek(fd,0,SEEK_SET);									// Set the offset to beginning
	while (size != buf_size)					
	{	
		size = read(fd,buf,buf_size);						
		if (size == -1)
		{
			perror("Failed to read input file");
			free(filename);
			free(buf);
			close(fd);
			exit(1);
		}
	}


	char text[150];
	sprintf(text,"Florist application initializing from file: %s\n",filename); 
	write(STDOUT_FILENO,text,strlen(text));

	
	
	// Critical section
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(filename);
		free(buf);
		close(fd);
		exit(1);
	}
	free(filename);			// Free resources. We don't need them anymore
	filename = NULL;
	if (close(fd) == -1)
	{
		perror("Failed to close the descriptor of the input file");
		free(buf);
		exit(1);
	}
	isOpened = 0;
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(buf);
		exit(1);
	}
	// End of critical section


	// strtok can change original array, so I  made a copy of it
	char* bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));	
	char* temp = bufCpy;
	strcpy(bufCpy,buf);
	// Find the number of florists and save this information to threadCount
	char* token = strtok(bufCpy,";");
	while(token != NULL)
	{	
		++threadCount;
		token = strtok(NULL,";");
	}

	threadCount -= 1;
	free(temp);


	bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	temp = bufCpy;
	strcpy(bufCpy,buf);
	token = strtok(bufCpy,":");
	// Find the number of clients and save this variable into clientCount
	while(token != NULL)
	{
		++clientCount;
		token = strtok(NULL,":");
	}

	clientCount -= 1;
	clientCount -= threadCount;
	free(temp);


	char text2[100];
	sprintf(text2,"%d florists have been created\n",threadCount); 
	write(STDOUT_FILENO,text2,strlen(text2));



	// Critical seciton
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(buf);
		exit(1);
	}
	// Allocate space for florists
	florists = (struct florist*) malloc(sizeof(struct florist)*threadCount);


	count = 1;
	bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	temp = bufCpy;
	strcpy(bufCpy,buf);
	token = strtok(bufCpy,"(");
	florists[0].name = (char*) malloc(sizeof(char)*(strlen(token)+1));
	strcpy(florists[0].name,token);
	// Parse names of florists
	while(count != threadCount)
	{	
		token = strtok(NULL,"\n");
		token = strtok(NULL,"(");

		florists[count].name = (char*) malloc(sizeof(char)*(strlen(token)+1));
		strcpy(florists[count].name,token);
		++count;
	}
	free(temp);



	count = 0;
	bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	temp = bufCpy;
	strcpy(bufCpy,buf);
	token = strtok(bufCpy,"(");
	// Parse coordinates of florists
	while(count != threadCount)
	{	
		token = strtok(NULL,",");
		florists[count].x = atof(token);
		token = strtok(NULL,";");
		florists[count].y = atof(token);
		token = strtok(NULL,")");
		florists[count].speed = atof(token);
		++ count;
		token = strtok(NULL,"(");
	}
	free(temp);



	// Find the number of flower kinds that the florist sells
	int k = 0;
	count = 0;
	bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	temp = bufCpy;
	strcpy(bufCpy,buf);
	token = strtok_r(bufCpy,"\n",&bufCpy);
	char* token2 = strtok_r(token,":",&token);
	// Parse coordinates of florists
	while(count != threadCount)
	{	
		char* tkn = strtok_r(token,",",&token);
		while (tkn != NULL) {
			++k;
			tkn = strtok_r(token,",",&token);
		}

		florists[count].size = k;
		k = 0;
		++count;
		token = strtok_r(bufCpy,"\n",&bufCpy);
		token2 = strtok_r(token,":",&token);
	}
	free(temp);



	
	// Allocate space for flower types array
	for (int i = 0; i < threadCount; ++i)
	{
		florists[i].flowerTypes = (char**) malloc(sizeof(char*)*(florists[i].size));
	}

	count = 0;
	k = 0;
	bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	temp = bufCpy;
	strcpy(bufCpy,buf);
	token = strtok_r(bufCpy,"\n",&bufCpy);
	token2 = strtok_r(token,":",&token);
	// Parse flower types that florists sell
	while(count != threadCount)
	{	
		char* token3 = strtok_r(token,",",&token);
		while(token3 != NULL) {

			florists[count].flowerTypes[k] = (char*) malloc(sizeof(char)*(strlen(token3)+1));
			strcpy(florists[count].flowerTypes[k],token3);
			++k;
			token3 = strtok_r(token,",",&token);
		}

		k = 0;
		++count;
		token = strtok_r(bufCpy,"\n",&bufCpy);
		token2 = strtok_r(token,":",&token);
		
	}
	free(temp);
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);
		exit(1);
	}
	// End of critical section



	// Eliminate spaces from names
	for (int i = 0; i < threadCount; ++i)
	{
		char tmp[strlen(florists[i].name) + 1];
		strcpy(tmp,florists[i].name);
		count = 0;
		for (int j = 0; tmp[j] != '\0' ; ++j)
		{	
			if (tmp[j] != ' ')
			{
				florists[i].name[count++] = tmp[j];
			}
		}

		florists[i].name[count] = '\0';
	}


	// Eliminate spaces from flower names
	for (int i = 0; i < threadCount; ++i)
	{
		for (int j = 0; j < florists[i].size ; ++j)
		{	
			char tmp2[strlen(florists[i].flowerTypes[j]) + 1];
			strcpy(tmp2,florists[i].flowerTypes[j]);
			count = 0;
			for (int k = 0; tmp2[k] != '\0'; ++k)
			{
				if (tmp2[k] != ' ')
				{	
					florists[i].flowerTypes[j][count++] = tmp2[k];
				}

			}
			florists[i].flowerTypes[j][count] = '\0';
		}
	}


	// Critical section
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);
		exit(1);
	}
	// Create request Queue for every thread
	requests = (struct Queue*) malloc(sizeof(struct Queue)*threadCount);

	// Allocate space for request array
	for (int i = 0; i < threadCount; ++i)
	{
		requests[i].request = (char**) malloc(sizeof(char*)*clientCount);
		for (int j = 0; j < clientCount; ++j)
		{
			requests[i].request[j] = (char*) malloc(sizeof(char)*1000);
		}
	}

	// Set the initial offset
	for (int i = 0; i < threadCount; ++i)
	{
		requests[i].offset = 0;
	}

	for (int i = 0; i < threadCount; ++i)
	{
		requests[i].distance = (float*) malloc(sizeof(float)*clientCount);
	}

	for (int i = 0; i < threadCount; ++i)
	{
		requests[i].client_no = (char**) malloc(sizeof(char*)*clientCount);
		for (int j = 0; j < clientCount; ++j)
		{
			requests[i].client_no[j] = (char*) malloc(sizeof(char)*1000);
		}
	}
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);

		for (int i = 0; i < threadCount; ++i)
		{
			free(requests[i].distance);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].request[j]);
			}
			free(requests[i].request);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].client_no[j]);
			}
			free(requests[i].client_no);
		}
		free(requests);
		exit(1);
	}
	// End of critical seciton
	

	// To check if the request Queue is full
	Count = (int*) malloc(sizeof(int)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		Count[i] = 0;
	}

	// To check if the florists is done
	flag = (int*) malloc(sizeof(int)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		flag[i] = 0;
	}

	// To check number of orders which is delivered
	order = (int*) malloc(sizeof(int)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		order[i] = 0;
	}



	// Critical section
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Blocking operation is failed");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);

		for (int i = 0; i < threadCount; ++i)
		{
			free(requests[i].distance);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].request[j]);
			}
			free(requests[i].request);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].client_no[j]);
			}
			free(requests[i].client_no);
		}
		free(requests);

		free(Count);
		free(order);
		free(flag);
		exit(1);
	}

	// To understand how many mutexes are created. 
	//If an error occurs, I can cancel mutexes by this way.
	int createdM[threadCount]; 
	for (int i = 0; i < threadCount; ++i)
	{
		createdM[i] = 0;
	}
	// Initialize mutexes
	mutexes = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		if(pthread_mutex_init(&mutexes[i],NULL) != 0)
		{
			perror("Failed to initialize mutex");
			free(buf);
			for (int i = 0; i < threadCount; ++i)
			{
				free(florists[i].name);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < florists[i].size; ++j)
				{
					free(florists[i].flowerTypes[j]);
				}
				free(florists[i].flowerTypes);
			}
			free(florists);

			for (int i = 0; i < threadCount; ++i)
			{
				free(requests[i].distance);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].request[j]);
				}
				free(requests[i].request);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].client_no[j]);
				}
				free(requests[i].client_no);
			}
			free(requests);

			free(Count);
			free(order);
			free(flag);

			for (int j = 0; j < threadCount; ++j)
			{
				if (createdM[j] == 1)
					pthread_mutex_destroy(&mutexes[j]);
			}
			free(mutexes);

			exit(1);
		}

		createdM[i] = 1;
	}

	

	int createdC[threadCount]; 
	for (int i = 0; i < threadCount; ++i)
	{
		createdC[i] = 0;
	}
	full = (pthread_cond_t*) malloc(sizeof(pthread_cond_t)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		if(pthread_cond_init(&full[i],NULL) != 0)
		{
			perror("Failed to initialize conditional variable");
			free(buf);
			for (int i = 0; i < threadCount; ++i)
			{
				free(florists[i].name);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < florists[i].size; ++j)
				{
					free(florists[i].flowerTypes[j]);
				}
				free(florists[i].flowerTypes);
			}
			free(florists);

			for (int i = 0; i < threadCount; ++i)
			{
				free(requests[i].distance);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].request[j]);
				}
				free(requests[i].request);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].client_no[j]);
				}
				free(requests[i].client_no);
			}
			free(requests);

			free(Count);
			free(order);
			free(flag);

			for (int j = 0; j < threadCount; ++j)
			{
				pthread_mutex_destroy(&mutexes[j]);
			}
			free(mutexes);

			for (int j = 0; j < threadCount; ++j)
			{
				if (createdC[j] == 1)
					pthread_cond_destroy(&full[j]);
			}
			free(full);

			exit(1);
		}

		createdC[i] = 1;
	}



	int createdC2[threadCount]; 
	for (int i = 0; i < threadCount; ++i)
	{
		createdC2[i] = 0;
	}
	isFinished = (pthread_cond_t*) malloc(sizeof(pthread_cond_t)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		if(pthread_cond_init(&isFinished[i],NULL) != 0)
		{
			perror("Failed to initialize conditional variable");
			free(buf);
			for (int i = 0; i < threadCount; ++i)
			{
				free(florists[i].name);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < florists[i].size; ++j)
				{
					free(florists[i].flowerTypes[j]);
				}
				free(florists[i].flowerTypes);
			}
			free(florists);

			for (int i = 0; i < threadCount; ++i)
			{
				free(requests[i].distance);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].request[j]);
				}
				free(requests[i].request);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].client_no[j]);
				}
				free(requests[i].client_no);
			}
			free(requests);

			free(Count);
			free(order);
			free(flag);

			for (int j = 0; j < threadCount; ++j)
			{
				pthread_mutex_destroy(&mutexes[j]);
			}
			free(mutexes);

			for (int j = 0; j < threadCount; ++j)
			{
				pthread_cond_destroy(&full[j]);
			}
			free(full);

			for (int j = 0; j < threadCount; ++j)
			{
				if (createdC2[j] == 1)
					pthread_cond_destroy(&isFinished[j]);
			}
			free(isFinished);

			exit(1);
		}

		createdC2[i] = 1;
	}



	// Parameters to use in creating threads
	// Indicates florist index
	param = (void**) malloc(sizeof(void*)*threadCount);
	for (int i = 0; i < threadCount; ++i)
	{
		param[i] = (void*) malloc(sizeof(void)*1);
	}
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);

		for (int i = 0; i < threadCount; ++i)
		{
			free(requests[i].distance);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].request[j]);
			}
			free(requests[i].request);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].client_no[j]);
			}
			free(requests[i].client_no);
		}
		free(requests);

		free(Count);
		free(order);
		free(flag);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_mutex_destroy(&mutexes[j]);
		}
		free(mutexes);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&full[j]);
		}
		free(full);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&isFinished[j]);
		}
		free(isFinished);

		for (int i = 0; i < threadCount; ++i)
		{
			free(param[i]);
		}
		free(param);
		
		exit(1);
	}
	// end of critical section



	for (int i = 0; i < threadCount; ++i)
	{
		*((int*) ((int**) param)[i])  = i;	
	}


	

	// To understand how many threads are created. 
	//If an error occurs, ı can cancel threads by this way.
	int created[threadCount]; 
	for (int i = 0; i < threadCount; ++i)
	{
		created[i] = 0;
	}
	// Create threads
	// Every thread corresponds a a florist.
	pthread_t threadIds[threadCount];
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);

		for (int i = 0; i < threadCount; ++i)
		{
			free(requests[i].distance);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].request[j]);
			}
			free(requests[i].request);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].client_no[j]);
			}
			free(requests[i].client_no);
		}
		free(requests);

		free(Count);
		free(order);
		free(flag);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_mutex_destroy(&mutexes[j]);
		}
		free(mutexes);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&full[j]);
		}
		free(full);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&isFinished[j]);
		}
		free(isFinished);

		for (int i = 0; i < threadCount; ++i)
		{
			free(param[i]);
		}
		free(param);
		exit(1);
	}

	for (int i = 0; i < threadCount; ++i)
	{	
		if (pthread_create(&threadIds[i],NULL,floristFunction,param[i]) != 0)
		{
			perror("Failed to create thread");
			for (int j = 0; j < threadCount; ++j)
			{
				if (created[j] == 1)
					pthread_cancel(threadIds[j]);
			}

			for (int j = 0; j < threadCount; ++j)
			{
				if (created[j] == 1)
					pthread_join(threadIds[j],NULL);
			}

			free(buf);
			for (int i = 0; i < threadCount; ++i)
			{
				free(florists[i].name);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < florists[i].size; ++j)
				{
					free(florists[i].flowerTypes[j]);
				}
				free(florists[i].flowerTypes);
			}
			free(florists);

			for (int i = 0; i < threadCount; ++i)
			{
				free(requests[i].distance);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].request[j]);
				}
				free(requests[i].request);
			}
			for (int i = 0; i < threadCount; ++i)
			{
				for (int j = 0; j < clientCount; ++j)
				{
					free(requests[i].client_no[j]);
				}
				free(requests[i].client_no);
			}
			free(requests);

			free(Count);
			free(order);
			free(flag);

			for (int j = 0; j < threadCount; ++j)
			{
				pthread_mutex_destroy(&mutexes[j]);
			}
			free(mutexes);

			for (int j = 0; j < threadCount; ++j)
			{
				pthread_cond_destroy(&full[j]);
			}
			free(full);

			for (int j = 0; j < threadCount; ++j)
			{
				pthread_cond_destroy(&isFinished[j]);
			}
			free(isFinished);

			for (int i = 0; i < threadCount; ++i)
			{
				free(param[i]);
			}
			free(param);
			exit(1);
		}

		created[i] = 1;
	}

	

 
	write(STDOUT_FILENO,"Processing requests\n",21);

	// Set the cursor to beginning of clients
	bufCpy = (char*) malloc(sizeof(char)*(buf_size+1));
	temp = bufCpy;
	strcpy(bufCpy,buf);
	char* token_ = strtok_r(bufCpy,"\n",&bufCpy);
	for (int i = 1; i < threadCount; ++i)
	{
		token_ = strtok_r(bufCpy,"\n",&bufCpy);
	}

	float x, y, dis;
	// Parse flower types that clients request
	token_ = strtok_r(bufCpy,"\n",&bufCpy);

	while(token_!=NULL)
	{	

		char* clientNum = strtok_r(token_,"(",&token_);
		token2 = strtok_r(token_,",",&token_);
		x = atof(token2);
		token2 = strtok_r(token_,")",&token_);
		y = atof(token2);
		token2 = strtok_r(token_,":",&token_);

		// Elimimate space
		char* tmp3 = (char*) malloc(sizeof(char)*(strlen(token2)+1));
		int cnt = 0;
		for (int i = 0; token2[i] != '\0' ; ++i)
		{	
			if (token2[i] != ' ')
			{
				tmp3[cnt++] = token2[i];
			}
		}
		
		tmp3[cnt] = '\0';
		
		// Go to the bottom line
		token_ = strtok_r(bufCpy,"\n",&bufCpy);
	
		

		// If florist sells the requested flower, find the distance
		int no = -1; 		// to indicate index of request queue
		float min = 0; 		// to find min distance;
		k = 0;



		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{	
				
				// If current florist sells the requested flower, find the distance and compare with min value
				if (strcmp(florists[i].flowerTypes[j],tmp3) == 0)
				{	
					dis = findChebyshevDistance(x,y,florists[i].x,florists[i].y);
					if (k == 0){
						min = dis;
						no = i;
					}
					
					if (k > 0 && min > dis){
						min = dis;
						no = i;
					}

					++k;
					j = florists[i].size;
				}
			}
		}


		// Add flower into request queue and set the offset
		pthread_mutex_lock(&mutexes[no]);
		strcpy(requests[no].request[requests[no].offset],tmp3);
		requests[no].distance[requests[no].offset] =  min;
		strcpy(requests[no].client_no[requests[no].offset],clientNum);
		requests[no].offset +=  1;
		++Count[no];
		pthread_cond_signal(&full[no]);
		pthread_mutex_unlock(&mutexes[no]);
		free(tmp3);
		
	}
	free(temp);



	// Central thread is done, but florists may be unfinished. So wait for them.
	// Wait for all florists to deliver all orders 
	for (int i = 0; i < threadCount; ++i)
	{	
		pthread_mutex_lock(&mutexes[i]);
		while(order[i] != requests[i].offset) {
			pthread_cond_wait(&isFinished[i],&mutexes[i]);
		}
		pthread_mutex_unlock(&mutexes[i]);
	}

	write(STDOUT_FILENO,"All requests processed\n",24);

	// When all the order are delivered, central thread inform florists that there are no more orders
	// Florist waits for a signal to continue, they are stuck in the loop, 
	// because all requests are delivered and request array is empty. 
	for (int i = 0; i < threadCount; ++i)
	{	
		pthread_mutex_lock(&mutexes[i]);
		flag[i] = 1;
		++Count[i];
		pthread_cond_signal(&full[i]);
		pthread_mutex_unlock(&mutexes[i]);
	}



	struct Statistic** Tmp = (struct Statistic**) malloc(sizeof(struct Statistic*)*threadCount);
	// Wait for all worker threads to prevent threads being zombie and get their return value
	for (int i = 0; i < threadCount; ++i)
	{
		if (pthread_join(threadIds[i],(void*) &Tmp[i]) != 0)
		{
			perror("Can't join other threads");
			exit(1);
		}
	}

	// threads is done, if someone sent sigint signal, now program can be interrupted by freeing resources 
	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);

		for (int i = 0; i < threadCount; ++i)
		{
			free(requests[i].distance);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].request[j]);
			}
			free(requests[i].request);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].client_no[j]);
			}
			free(requests[i].client_no);
		}
		free(requests);

		free(Count);
		free(order);
		free(flag);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_mutex_destroy(&mutexes[j]);
		}
		free(mutexes);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&full[j]);
		}
		free(full);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&isFinished[j]);
		}
		free(isFinished);

		for (int i = 0; i < threadCount; ++i)
		{
			free(param[i]);
		}
		free(param);
		exit(1);
	}



	// Print statistics on the screen
	printStatics(Tmp,florists,threadCount);

	


	// Free resources - critical section
	if (sigprocmask(SIG_BLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		free(buf);
		for (int i = 0; i < threadCount; ++i)
		{
			free(florists[i].name);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < florists[i].size; ++j)
			{
				free(florists[i].flowerTypes[j]);
			}
			free(florists[i].flowerTypes);
		}
		free(florists);

		for (int i = 0; i < threadCount; ++i)
		{
			free(requests[i].distance);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].request[j]);
			}
			free(requests[i].request);
		}
		for (int i = 0; i < threadCount; ++i)
		{
			for (int j = 0; j < clientCount; ++j)
			{
				free(requests[i].client_no[j]);
			}
			free(requests[i].client_no);
		}
		free(requests);

		free(Count);
		free(order);
		free(flag);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_mutex_destroy(&mutexes[j]);
		}
		free(mutexes);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&full[j]);
		}
		free(full);

		for (int j = 0; j < threadCount; ++j)
		{
			pthread_cond_destroy(&isFinished[j]);
		}
		free(isFinished);

		for (int i = 0; i < threadCount; ++i)
		{
			free(param[i]);
		}
		free(param);
		exit(1);
	}

	free(buf);
	buf = NULL;
	free(Count);
	Count = NULL;
	free(flag);
	flag = NULL;
	free(order);
	order = NULL;
	
	for (int i = 0; i < threadCount; ++i)
	{
		free(Tmp[i]);
	}
	free(Tmp);
	Tmp = NULL;
	
	for (int i = 0; i < threadCount; ++i)
	{		
		free(florists[i].name);
	}

	for (int i = 0; i < threadCount; ++i)
	{
		for (int j = 0; j < florists[i].size; ++j)
		{
			free(florists[i].flowerTypes[j]);
		}
		free(florists[i].flowerTypes);
	}
	free(florists);
	florists = NULL;

	for (int k = 0; k < threadCount; ++k)
	{
		for (int j = 0; j < clientCount; ++j)
		{
			free(requests[k].request[j]);
		}
		free(requests[k].request);
	}

	for (int j = 0; j < threadCount; ++j)
	{
		free(requests[j].distance);
	}

	for (int k = 0; k < threadCount; ++k)
	{
		for (int j = 0; j < clientCount; ++j)
		{
			free(requests[k].client_no[j]);
		}
		free(requests[k].client_no);
	}
	free(requests);
	requests = NULL;

	for (int i = 0; i < threadCount; ++i)
	{
		pthread_mutex_destroy(&mutexes[i]);
	}
	free(mutexes);
	mutexes = NULL;

	for (int j = 0; j < threadCount; ++j)
	{
		pthread_cond_destroy(&full[j]);
	}
	free(full);
	full = NULL;

	for (int j = 0; j < threadCount; ++j)
	{
		pthread_cond_destroy(&isFinished[j]);
	}
	free(isFinished);
	isFinished = NULL;

	for (int i = 0; i < threadCount; ++i)
	{
		free(param[i]);
	}
	free(param);
	param = NULL;

	if (sigprocmask(SIG_UNBLOCK,&set,NULL) == -1)
	{
		perror("Failed to unblock operation");
		exit(1);
	}
	// End of critical section


	return 0;
}