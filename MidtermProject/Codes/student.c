#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include "helper.h"


sem_t* fullCounter;
sem_t* emptyCounter;
sem_t* soup_atCounter;
sem_t* desert_atCounter;
sem_t* mainCourse_atCounter;
sem_t* table;
sem_t* flag;
int* numberOfStudents;

int fd1, fd2, fd3, fd4 ,fd5, fd7, fd8, fd9;

// To Catch SIGINT signal
void handler(int signo)
{	
	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);
	close(fd5);
	close(fd7);
	close(fd8);
	close(fd9);
	exit(1);
}

// Open shared memory
void openExistingObject(int t)
{

	fd1 = shm_open("shared3",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);	// empty counter
	if (fd1 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd2 = shm_open("shared4",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);	// full counter
	if (fd2 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd3 = shm_open("sharedsoupCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // soup at counter
	if (fd3 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd4 = shm_open("shareddesertCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // desert at counter
	if (fd4 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd5 = shm_open("sharedmainCourseCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // main course at counter
	if (fd5 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd7 = shm_open("shared19",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd7 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd8 = shm_open("shared20",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd8 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd9 = shm_open("shared21",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd9 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}


	emptyCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
	if (emptyCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	fullCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
	if (fullCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	soup_atCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);
	if (soup_atCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	desert_atCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd4, 0);
	if (desert_atCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	mainCourse_atCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd5, 0);
	if (mainCourse_atCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	table = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd7, 0);
	if (table == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	flag = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd8, 0);
	if (flag == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}

	numberOfStudents = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd9, 0);
	if (numberOfStudents == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}



	// Close file descriptors, they are no longer necessary
	if(close(fd1) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd2) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd3) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd4) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd5) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd7) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd8) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd9) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}	
}




int main(int argc, char const *argv[])
{

	// For SIGINT
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = &handler;

	// To catch SIGINT
	if (sigaction(SIGINT,&sa,NULL) == -1)
	{
		perror("sigaction error is occured");
		exit(1);
	}

	
	// Save command line arguments
	int order = atoi(argv[1]);
	int L = atoi(argv[2]);
	int t_size = atoi(argv[3]);
	int _no, val, l = 0;

	// Open shared memory
	openExistingObject(t_size);
	
	// Continue until a student eats a total of L.
	while(l != L)
	{	
		student_waitingForFood(order,l+1,numberOfStudents,soup_atCounter,mainCourse_atCounter,desert_atCounter);
		*numberOfStudents = *numberOfStudents + 1; // Increase the number of students that waits for food.

		// If all three of them is available, take them. Otherwise wait for a cook to supply plates.
		sem_wait(flag);					
		sem_wait(soup_atCounter);		// take soup
		sem_wait(desert_atCounter);		// take desert
		sem_wait(mainCourse_atCounter);	// take main course
		sem_wait(fullCounter);			// Decrease the space on the counter three times, because students take three 
		sem_wait(fullCounter);			// plates at the same time. 
		sem_wait(fullCounter);		
		
		student_wait_getTable(order,l+1,table);

		// S/he is going to the table, is not waiting in front of the counter
		// So, decrese the number of the students that waits for food.
		*numberOfStudents = *numberOfStudents - 1; 
		sem_wait(table);						// Sit at a table, if none of them is available wait for a table to be empty.
		sem_getvalue(table,&val);
		_no = t_size - val;

		student_sittingToEat(order,_no,l+1,table);	// Student took food, sat at the table and eat it. So increase the counter.
		++l;

		sem_post(emptyCounter);					// Increase the space on the counter
		sem_post(emptyCounter);
		sem_post(emptyCounter);

		sem_post(table);						// Increase the number of available tables.
		student_goingAgainToCounter(order,_no,l+1,table);
	}

	student_finishingEating(order,L);

	return 0;
}