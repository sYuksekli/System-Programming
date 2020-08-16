#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "helper.h"



char* ingredients;
char* buf;
void* mf;
void* mw;
void* ms;
void* fw;
void* fs;
void* ws;
int semid;



// free resources
// I wrote this function to get rid of code clutter, because it was repeated too much.
void exitGracefully()
{
	free(ingredients);
	free(mf);
	free(mw);	
	free(ms);
	free(fw);
	free(fs);
	free(ws);
	free(buf);
	semctl(semid,5,IPC_RMID);
	exit(1);
}


// Cook function
void* cookFunction(void* parameters)
{	
	
	int cook = *((int*) parameters);									// Get the number of the chef

	if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL) != 0)	// Set cancel type
	{
		perror("Failed to set cancel type");
		exitGracefully();
	}


	srand(time(NULL));
	int random;


	// Set the operations.
	struct sembuf sop[1] ,sop1[2], sop2[2], sop3[2], sop4[2], sop5[2], sop6[2];

	sop[0].sem_num = 0;			// post(güllaç)	
	sop[0].sem_op = 1;
	sop[0].sem_flg = 0;

	sop1[0].sem_num = 1;		// wait(milk,flour) at the same time
	sop1[0].sem_op = -1;
	sop1[0].sem_flg = 0;
	sop1[1].sem_num = 2;
	sop1[1].sem_op = -1;
	sop1[1].sem_flg = 0;

	sop2[0].sem_num = 1;		// wait(milk,walnut) at the same time
	sop2[0].sem_op = -1;
	sop2[0].sem_flg = 0;
	sop2[1].sem_num = 3;
	sop2[1].sem_op = -1;
	sop2[1].sem_flg = 0;

	sop3[0].sem_num = 1;		// wait(milk,sugar) at the same time
	sop3[0].sem_op = -1;
	sop3[0].sem_flg = 0;
	sop3[1].sem_num = 4;
	sop3[1].sem_op = -1;
	sop3[1].sem_flg = 0;

	sop4[0].sem_num = 2;		// wait(flour,walnut) at the same time
	sop4[0].sem_op = -1;		
	sop4[0].sem_flg = 0;
	sop4[1].sem_num = 3;
	sop4[1].sem_op = -1;
	sop4[1].sem_flg = 0;

	sop5[0].sem_num = 2;		// wait(flour,sugar) at the same time
	sop5[0].sem_op = -1;
	sop5[0].sem_flg = 0;
	sop5[1].sem_num = 4;
	sop5[1].sem_op = -1;
	sop5[1].sem_flg = 0;

	sop6[0].sem_num = 3;		// wait(walnut,sugar) at the same time
	sop6[0].sem_op = -1;
	sop6[0].sem_flg = 0;
	sop6[1].sem_num = 4;
	sop6[1].sem_op = -1;
	sop6[1].sem_flg = 0;


 	while(1)
	{	

		if (cook == 1) {								// cook1 is waiting for flour and milk

			cook_isWaiting(1,"milk","flour");		
			if (semop(semid,sop1,2) == -1)				// wait(Milk,flour)
			{
				perror("Failed to wait milk and flour");
				exitGracefully();
			}
			
			cook_hasTaken(1,"milk");					// take the elements and mark the array as taken.
			ingredients[0] = 'x';
			cook_hasTaken(1,"flour");	
			ingredients[1] = 'x';

			random = rand() % 5 + 1;					// sleep for a random seconds to prepare dessert
			cook_isPreparing(1);
			sleep(random);
			cook_hasDelivered(1);
		}
		

		if (cook == 2) {								// cook2 is waiting for walnut and milk

			cook_isWaiting(2,"milk","walnuts");
			if (semop(semid,sop2,2) == -1)				// wait(walnut,milk)
			{
				perror("Failed to wait milk and walnut");
				exitGracefully();
			}

			cook_hasTaken(2,"milk");	
			ingredients[0] = 'x';
			cook_hasTaken(2,"walnuts");	
			ingredients[1] = 'x';

			random = rand() % 5 + 1;					// sleep for a random seconds to prepare dessert
			cook_isPreparing(2);
			sleep(random);
			cook_hasDelivered(2);
		}
		

		if (cook == 3) {								// cook3 is waiting for sugar and milk

			cook_isWaiting(3,"milk","sugar");	
			if (semop(semid,sop3,2) == -1)				// wait(sugar,milk)
			{
				perror("Failed to wait sugar and milk");
				exitGracefully();
			}

			cook_hasTaken(3,"milk");	
			ingredients[0] = 'x';
			cook_hasTaken(3,"sugar");	
			ingredients[1] = 'x';

			random = rand() % 5 + 1;
			cook_isPreparing(3);
			sleep(random);								// sleep for a random seconds to prepare dessert
			cook_hasDelivered(3);
		}
			

		if (cook == 4) {								// cook4 is waiting for flour and walnut

			cook_isWaiting(4,"flour","walnuts");
			if (semop(semid,sop4,2) == -1)				// wait(flour,walnut)
			{
				perror("Failed to flour and walnut");
				exitGracefully();
			}

			cook_hasTaken(4,"flour");	
			ingredients[0] = 'x';
			cook_hasTaken(4,"walnuts");	
			ingredients[1] = 'x';

			random = rand() % 5 + 1;
			cook_isPreparing(4);
			sleep(random);								// sleep for a random seconds to prepare dessert
			cook_hasDelivered(4);
		}
		

		if (cook == 5) {								// cook5 is waiting for flour and sugar

			cook_isWaiting(5,"flour","sugar");
			if (semop(semid,sop5,2) == -1)				// wait(flour,sugar)
			{
				perror("Failed to wait flour and sugar");
				exitGracefully();
			}

			cook_hasTaken(5,"flour");	
			ingredients[0] = 'x';
			cook_hasTaken(5,"sugar");	
			ingredients[1] = 'x';

			random = rand() % 5 + 1;
			cook_isPreparing(5);
			sleep(random);								// sleep for a random seconds to prepare dessert
			cook_hasDelivered(5);
		}
		

		if (cook == 6) {								// cook6 is waiting for walnut and sugar

			cook_isWaiting(6,"walnuts","sugar");
			if (semop(semid,sop6,2) == -1)				// wait(walnut,sugar)
			{
				perror("Failed to wait walnut and sugar");
				exitGracefully();
			}

			cook_hasTaken(6,"walnuts");	
			ingredients[0] = 'x';
			cook_hasTaken(6,"sugar");	
			ingredients[1] = 'x';

			random = rand() % 5 + 1;
			cook_isPreparing(6);
			sleep(random);								// sleep for a random seconds to prepare dessert
			cook_hasDelivered(6);
		}
		

		if (semop(semid,sop,1) == -1)					// post(güllaç)
		{
			perror("Failed to post güllaç");
			exitGracefully();
		}
	}

	return NULL;
}




int main(int argc, char *argv[])
{	


	ingredients = (char*) malloc(sizeof(char)*2);	// Data structure to hold ingredients

	mf = (void*) malloc(sizeof(void)*1);			// mf, mw, ms, fw, fs, ws are parameters of thread function.								// It shows the number of the chef.  
	*((int*) mf) = 1;								// They shows the number of the cooks.

	mw = (void*) malloc(sizeof(void)*1);
	*((int*) mw) = 2;

	ms = (void*) malloc(sizeof(void)*1);
	*((int*) ms) = 3;

	fw = (void*) malloc(sizeof(void)*1);
	*((int*) fw) = 4;

	fs = (void*) malloc(sizeof(void)*1);
	*((int*) fs) = 5;

	ws = (void*) malloc(sizeof(void)*1);
	*((int*) ws) = 6;

	
	// count is the number of command line arguments
	int count;
	char* filename = (char*) malloc(sizeof(char)*100);
	count = parseCommandLine(argc, argv, filename);
	if (count != 1)
	{
		write(STDOUT_FILENO,"You entered missing command lines, You must enter a command line like this : -i filePath\n",90);
		free(filename);
		free(ingredients);
		free(mf);
		free(mw);
		free(ms);
		free(fw);
		free(fs);
		free(ws);
		exit(1);
	}


	int i, fd, size = -1;
	

	// Open the input file in read only mode
	fd = open(filename,O_RDONLY);		
	if (fd == -1)
	{
		perror("Failed to open input file");
		free(filename);
		free(ingredients);
		free(mf);
		free(mw);
		free(ms);
		free(fw);
		free(fs);
		free(ws);
		exit(1);
	}


	int  buf_size = lseek(fd,0,SEEK_END);	// get the size of the file
	if (buf_size < 29)						// It must be at least 10 rows.
	{
		write(STDOUT_FILENO,"You must enter at least 10 rows\n",33);
		free(filename);
		free(ingredients);
		free(mf);
		free(mw);
		free(ms);
		free(fw);
		free(fs);
		free(ws);
		close(fd);
		exit(1);
	}

	buf = (char*) malloc(sizeof(char)*(buf_size));	// Read file into an array

	lseek(fd,0,SEEK_SET);							// Set the offset. 
	while (size != buf_size)					
	{	
		size = read(fd,buf,buf_size);						
		if (size == -1)
		{
			perror("Failed to read input file");
			free(filename);
			free(ingredients);
			free(mf);
			free(mw);
			free(ms);
			free(fw);
			free(fs);
			free(ws);
			close(fd);
			free(buf);
			exit(1);
		}
	}
	
	free(filename);			// Free resouces. We don't need them anymore
	if (close(fd) == -1)
	{
		perror("Failed to close the descriptor of the input file");
		free(ingredients);
		free(mf);
		free(mw);
		free(ms);
		free(fw);
		free(fs);
		free(ws);
		free(buf);
		exit(1);
	}

	// Check if every character is correct. If the character that is read from file is not W, F, S or M exit.
	// Also, every line has to have 2 characters. Check it. If it is wrong, exit. 
	for (i = 2; i < buf_size; i+=3)
	{
		if (buf[i] != '\n')
		{
			write(STDOUT_FILENO,"Content of the file is invalid. You must enter like \nMF\nMS\nWS...\n",69);
			free(ingredients);
			free(mf);
			free(mw);
			free(ms);
			free(fw);
			free(fs);
			free(ws);
			free(buf);
			exit(1);
		}
	}

	for (i = 0; i < buf_size; ++i)
	{
		if (!(buf[i] == 'M' || buf[i] == 'F' || buf[i] == 'S' || buf[i] == 'W' || buf[i] == 'm' || buf[i] == 'f' || buf[i] == 's' || buf[i] == 'w'|| buf[i] == '\n'))
		{
			char text[100];
			sprintf(text,"You entered invalid character : %c. You must enter M, S, F or W.\n",buf[i]);
			write(STDOUT_FILENO,text,strlen(text));
			free(ingredients);
			free(mf);
			free(mw);
			free(ms);
			free(fw);
			free(fs);
			free(ws);
			free(buf);
			exit(1);
		}
	}

	// If user enters some line like MM , FF...
	for (i = 0; i < buf_size; i+=3)
	{
		if (buf[i] == buf[i+1])
		{
			write(STDOUT_FILENO,"Content of the file is invalid. You must enter like \nMF\nMS\nWS...\n",69);
			free(ingredients);
			free(mf);
			free(mw);
			free(ms);
			free(fw);
			free(fs);
			free(ws);
			free(buf);
			exit(1);
		}
	}



	union semun arg1, arg2;

	// Create semaphore set
	semid = semget(IPC_PRIVATE,5,IPC_CREAT | S_IRUSR | S_IWUSR);	// Food semaphores, milk, flour, sugar, walnut, güllaç
	if (semid == -1)
	{
		perror("Can't create semaphore set");
		free(ingredients);
		free(mf);
		free(mw);
		free(ms);
		free(fw);
		free(fs);
		free(ws);
		free(buf);
		exit(1);
	}


	arg1.val = 1;  // initial value of  güllaç
	arg2.val = 0;  // initial value of milk, flour, wallnut, sugar

	// Set their initial values
	if (semctl(semid,0,SETVAL,arg1) == -1)	// güllaç
	{
	 	perror("Can't initialize güllaç semaphore");
	 	exitGracefully();
	} 

	if (semctl(semid,1,SETVAL,arg2) == -1)	// milk
	{
	 	perror("Can't initialize milk semaphore");
	 	exitGracefully();
	} 
	if (semctl(semid,2,SETVAL,arg2) == -1)	// flour
	{
	 	perror("Can't initialize flour semaphore");
	 	exitGracefully();
	} 
	if (semctl(semid,3,SETVAL,arg2) == -1)	// walnut
	{
	 	perror("Can't initialize walnut semaphore");
	 	exitGracefully();
	} 
	if (semctl(semid,4,SETVAL,arg2) == -1)	// sugar
	{
	 	perror("Can't initialize sugar semaphore");
	 	exitGracefully();
	} 
	

	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0)
	{
		perror("Failed to initialize thread attribute");
		exitGracefully();
	}
	if (pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE) != 0)
	{
		perror("Failed to set thread attribute");
		pthread_attr_destroy(&attr);
		exitGracefully();
	}


	// Create threads
	// Every thread corresponds a a chef.
	pthread_t t_cook1, t_cook2, t_cook3, t_cook4, t_cook5, t_cook6;
	if (pthread_create(&t_cook1,&attr,cookFunction,mf) != 0)
	{
		perror("Failed to create thread");
		exitGracefully();
	}
	if (pthread_create(&t_cook2,&attr,cookFunction,mw) != 0)
	{
		perror("Failed to create thread");
		pthread_cancel(t_cook1);
		pthread_join(t_cook1,NULL);
		exitGracefully();
	}
	if (pthread_create(&t_cook3,&attr,cookFunction,ms) != 0)
	{
		perror("Failed to create thread");
		pthread_cancel(t_cook1);
		pthread_cancel(t_cook2);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		exitGracefully();
	}
	if (pthread_create(&t_cook4,&attr,cookFunction,fw) != 0)
	{
		perror("Failed to create thread");
		pthread_cancel(t_cook1);
		pthread_cancel(t_cook2);
		pthread_cancel(t_cook3);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		exitGracefully();
	}
	if (pthread_create(&t_cook5,&attr,cookFunction,fs) != 0)
	{
		perror("Failed to create thread");
		pthread_cancel(t_cook1);
		pthread_cancel(t_cook2);
		pthread_cancel(t_cook3);
		pthread_cancel(t_cook4);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		exitGracefully();
	}
	if (pthread_create(&t_cook6,&attr,cookFunction,ws) != 0)
	{
		perror("Failed to create thread");
		pthread_cancel(t_cook1);
		pthread_cancel(t_cook2);
		pthread_cancel(t_cook3);
		pthread_cancel(t_cook4);
		pthread_cancel(t_cook5);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		exitGracefully();
	}

	if (pthread_attr_destroy(&attr) != 0)
	{
		perror("Failed to destroy thread attribute");
		pthread_cancel(t_cook1);
		pthread_cancel(t_cook2);
		pthread_cancel(t_cook3);
		pthread_cancel(t_cook4);
		pthread_cancel(t_cook5);
		pthread_cancel(t_cook6);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	


	struct  sembuf sops[1] ,sops1[2], sops2[2], sops3[2], sops4[2], sops5[2], sops6[2];

	sops[0].sem_num = 0;	// To wait(güllaç)
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;

	sops1[0].sem_num = 1;	// To post(milk,flour) at the same time
	sops1[0].sem_op = 1;
	sops1[0].sem_flg = 0;
	sops1[1].sem_num = 2;
	sops1[1].sem_op = 1;
	sops1[1].sem_flg = 0;

	sops2[0].sem_num = 1;	// To post(milk,walnut) at the same time
	sops2[0].sem_op = 1;
	sops2[0].sem_flg = 0;
	sops2[1].sem_num = 3;
	sops2[1].sem_op = 1;
	sops2[1].sem_flg = 0;

	sops3[0].sem_num = 1; 	// To post(milk,sugar) at the same time
	sops3[0].sem_op = 1;
	sops3[0].sem_flg = 0;
	sops3[1].sem_num = 4;
	sops3[1].sem_op = 1;
	sops3[1].sem_flg = 0;

	sops4[0].sem_num = 2;	// To post(flour,walnut) at the same time
	sops4[0].sem_op = 1;
	sops4[0].sem_flg = 0;
	sops4[1].sem_num = 3;
	sops4[1].sem_op = 1;
	sops4[1].sem_flg = 0;

	sops5[0].sem_num = 2;	// To post(flour,sugar) at the same time
	sops5[0].sem_op = 1;
	sops5[0].sem_flg = 0;
	sops5[1].sem_num = 4;
	sops5[1].sem_op = 1;
	sops5[1].sem_flg = 0;

	sops6[0].sem_num = 3;	// To post(walnut,sugar) at the same time
	sops6[0].sem_op = 1;
	sops6[0].sem_flg = 0;
	sops6[1].sem_num = 4;
	sops6[1].sem_op = 1;
	sops6[1].sem_flg = 0;



	i = 0;
	char ing1, ing2;
	while(i < buf_size)		
	{
		if (semop(semid,sops,1) == -1)	// wait(güllaç)
		{
			perror("Failed to wait güllaç semaphore");
			pthread_cancel(t_cook1);
			pthread_cancel(t_cook2);
			pthread_cancel(t_cook3);
			pthread_cancel(t_cook4);
			pthread_cancel(t_cook5);
			pthread_cancel(t_cook6);
			pthread_join(t_cook1,NULL);
			pthread_join(t_cook2,NULL);
			pthread_join(t_cook3,NULL);
			pthread_join(t_cook4,NULL);
			pthread_join(t_cook5,NULL);
			pthread_join(t_cook6,NULL);
			exitGracefully();
		}


		if (i != 0)	// Take the dessert 
		{
			wholesaler_Obtained();
		}
		

		ingredients[0] = buf[i];	// Save the current ingredients into the ingredients array so that chefs can see them.
		ingredients[1] = buf[i+1];
		ing1 = ingredients[0];
		ing2 = ingredients[1];

		if ((ing1 == 'M' && ing2 == 'F') || (ing1 == 'm' && ing2 == 'f') || (ing1 == 'f' && ing2 == 'm') || (ing1 == 'F' && ing2 == 'M'))
		{	

			wholesaler_Delivering("milk","flour");
			if (semop(semid,sops1,2) == -1)		// post(Milk,flour)
			{
				perror("Failed to post milk and flour");
				pthread_cancel(t_cook1);
				pthread_cancel(t_cook2);
				pthread_cancel(t_cook3);
				pthread_cancel(t_cook4);
				pthread_cancel(t_cook5);
				pthread_cancel(t_cook6);
				pthread_join(t_cook1,NULL);
				pthread_join(t_cook2,NULL);
				pthread_join(t_cook3,NULL);
				pthread_join(t_cook4,NULL);
				pthread_join(t_cook5,NULL);
				pthread_join(t_cook6,NULL);
				exitGracefully();
			}
		}

		if ((ing1 == 'M' && ing2 == 'W') || (ing1 == 'W' && ing2 == 'M') || (ing1 == 'm' && ing2 == 'w') || (ing1 == 'w' && ing2 == 'm'))
		{

			wholesaler_Delivering("milk","wallnuts");
			if (semop(semid,sops2,2) == -1)		// post(Milk,walnut)
			{
				perror("Failed to post milk and walnut");
				pthread_cancel(t_cook1);
				pthread_cancel(t_cook2);
				pthread_cancel(t_cook3);
				pthread_cancel(t_cook4);
				pthread_cancel(t_cook5);
				pthread_cancel(t_cook6);
				pthread_join(t_cook1,NULL);
				pthread_join(t_cook2,NULL);
				pthread_join(t_cook3,NULL);
				pthread_join(t_cook4,NULL);
				pthread_join(t_cook5,NULL);
				pthread_join(t_cook6,NULL);
				exitGracefully();
			}
		}

		if ((ing1 == 'M' && ing2 == 'S') || (ing1 == 'S' && ing2 == 'M') || (ing1 == 'm' && ing2 == 's') || (ing1 == 's' && ing2 == 'm'))
		{

			wholesaler_Delivering("milk","sugar");
			if (semop(semid,sops3,2) == -1)		// post(Milk,sugar)
			{
				perror("Failed to post milk and sugar");
				pthread_cancel(t_cook1);
				pthread_cancel(t_cook2);
				pthread_cancel(t_cook3);
				pthread_cancel(t_cook4);
				pthread_cancel(t_cook5);
				pthread_cancel(t_cook6);
				pthread_join(t_cook1,NULL);
				pthread_join(t_cook2,NULL);
				pthread_join(t_cook3,NULL);
				pthread_join(t_cook4,NULL);
				pthread_join(t_cook5,NULL);
				pthread_join(t_cook6,NULL);
				exitGracefully();
			}
		}

		if ((ing1 == 'F' && ing2 == 'W') || (ing1 == 'W' && ing2 == 'F') || (ing1 == 'f' && ing2 == 'w') || (ing1 == 'w' && ing2 == 'f'))
		{

			wholesaler_Delivering("flour","walnuts");
			if (semop(semid,sops4,2) == -1)		// post(flour,walnut)
			{
				perror("Failed to post flour and walnut");
				pthread_cancel(t_cook1);
				pthread_cancel(t_cook2);
				pthread_cancel(t_cook3);
				pthread_cancel(t_cook4);
				pthread_cancel(t_cook5);
				pthread_cancel(t_cook6);
				pthread_join(t_cook1,NULL);
				pthread_join(t_cook2,NULL);
				pthread_join(t_cook3,NULL);
				pthread_join(t_cook4,NULL);
				pthread_join(t_cook5,NULL);
				pthread_join(t_cook6,NULL);
				exitGracefully();
			}
		}

		if ((ing1 == 'F' && ing2 == 'S') || (ing1 == 'S' && ing2 == 'F') || (ing1 == 'f' && ing2 == 's') || (ing1 == 's' && ing2 == 'f'))
		{

			wholesaler_Delivering("flour","sugar");
			if (semop(semid,sops5,2) == -1)		// post(flour,sugar)
			{
				perror("Failed to post flour and sugar");
				pthread_cancel(t_cook1);
				pthread_cancel(t_cook2);
				pthread_cancel(t_cook3);
				pthread_cancel(t_cook4);
				pthread_cancel(t_cook5);
				pthread_cancel(t_cook6);
				pthread_join(t_cook1,NULL);
				pthread_join(t_cook2,NULL);
				pthread_join(t_cook3,NULL);
				pthread_join(t_cook4,NULL);
				pthread_join(t_cook5,NULL);
				pthread_join(t_cook6,NULL);
				exitGracefully();
			}
		}

		if ((ing1 == 'W' && ing2 == 'S') || (ing1 == 'S' && ing2 == 'W') || (ing1 == 'w' && ing2 == 's') || (ing1 == 's' && ing2 == 'w'))
		{

			wholesaler_Delivering("walnuts","sugar");
			if (semop(semid,sops6,2) == -1)		// post(walnut,sugar)
			{
				perror("Failed to walnut and sugar");
				pthread_cancel(t_cook1);
				pthread_cancel(t_cook2);
				pthread_cancel(t_cook3);
				pthread_cancel(t_cook4);
				pthread_cancel(t_cook5);
				pthread_cancel(t_cook6);
				pthread_join(t_cook1,NULL);
				pthread_join(t_cook2,NULL);
				pthread_join(t_cook3,NULL);
				pthread_join(t_cook4,NULL);
				pthread_join(t_cook5,NULL);
				pthread_join(t_cook6,NULL);
				exitGracefully();
			}
		}

		i += 3; 						// Don't take new line
		wholesaler_isWaiting();			// wholesaler is waiting for dessert to be ready.
	}


	if (semop(semid,sops,1) == -1)		// wait for rhe last güllaç
	{
		perror("Failed to wait güllaç semaphore");
		pthread_cancel(t_cook1);
		pthread_cancel(t_cook2);
		pthread_cancel(t_cook3);
		pthread_cancel(t_cook4);
		pthread_cancel(t_cook5);
		pthread_cancel(t_cook6);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}

	wholesaler_Obtained();				//The last sentence will not be appeared. So print it one more time.



	if (pthread_cancel(t_cook1) != 0)	// Inform other chefs that there will be no ingredients.
	{
		perror("Failed to cancel chef1 thread");
		pthread_cancel(t_cook2);
		pthread_cancel(t_cook3);
		pthread_cancel(t_cook4);
		pthread_cancel(t_cook5);
		pthread_cancel(t_cook6);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();

	}
	if (pthread_cancel(t_cook2) != 0)
	{
		perror("Failed to cancel chef2 thread");
		pthread_cancel(t_cook3);
		pthread_cancel(t_cook4);
		pthread_cancel(t_cook5);
		pthread_cancel(t_cook6);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();

	}
	if (pthread_cancel(t_cook3) != 0)
	{
		perror("Failed to cancel chef3 thread");
		pthread_cancel(t_cook4);
		pthread_cancel(t_cook5);
		pthread_cancel(t_cook6);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();

	}
	if (pthread_cancel(t_cook4) != 0)
	{
		perror("Failed to cancel chef4 thread");
		pthread_cancel(t_cook5);
		pthread_cancel(t_cook6);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	if (pthread_cancel(t_cook5) != 0)
	{
		perror("Failed to cancel chef5 thread");
		pthread_cancel(t_cook6);
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();

	}
	if (pthread_cancel(t_cook6) != 0)
	{
		perror("Failed to cancel chef6 thread");
		pthread_join(t_cook1,NULL);
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		exitGracefully();

	}
	


	// Wait for all worker threads
	if (pthread_join(t_cook1,NULL) != 0)
	{
		perror("Can't join other threads");
		pthread_join(t_cook2,NULL);
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	if (pthread_join(t_cook2,NULL) != 0)
	{
		perror("Can't join other threads");
		pthread_join(t_cook3,NULL);
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	if (pthread_join(t_cook3,NULL) != 0)
	{
		perror("Can't join other threads");
		pthread_join(t_cook4,NULL);
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	if (pthread_join(t_cook4,NULL) != 0)
	{
		perror("Can't join other threads");
		pthread_join(t_cook5,NULL);
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	if (pthread_join(t_cook5,NULL) != 0)
	{
		perror("Can't join other threads");
		pthread_join(t_cook6,NULL);
		exitGracefully();
	}
	if (pthread_join(t_cook6,NULL) != 0)
	{
		perror("Can't join other threads");
		exitGracefully();
	}
	



	//free resources
	free(ingredients);
	free(mf);
	free(mw);
	free(ms);
	free(fw);
	free(fs);
	free(ws);
	free(buf);
	if (semctl(semid,5,IPC_RMID) == -1)
	{
		perror("Failed to clean semaphore set");
		exit(1);
	}
	


	return 0;

}