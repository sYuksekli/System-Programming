#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include "helper.h"


sem_t* fullKitchen;
sem_t* emptyKitchen;
sem_t* fullCounter;
sem_t* emptyCounter;
sem_t* soup_atKitchen;
sem_t* desert_atKitchen;
sem_t* mainCourse_atKitchen;
sem_t* soup_atCounter;
sem_t* desert_atCounter;
sem_t* mainCourse_atCounter;
sem_t* mutexbetweenCooks;
sem_t* flag;
int* _index;
int* soup_value;
int* desert_value;
int* mainCourse_value;
int* s_counter;
int* d_counter;
int* m_counter;
int fd1, fd2, fd3, fd4 ,fd5, fd6, fd7, fd8, fd9, fd10, fd11, fd12, fd13, fd14, fd15, fd16, fd17, fd18, fd19;


// To Catch SIGINT signal
void handler(int signo)
{	
	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);
	close(fd5);
	close(fd6);
	close(fd7);
	close(fd8);
	close(fd9);
	close(fd10);
	close(fd11);
	close(fd12);
	close(fd13);
	close(fd14);
	close(fd15);
	close(fd16);
	close(fd17);
	close(fd18);
	close(fd19);
	exit(1);
}


// Decrease the number of plates in the kitchen
void takePlateFromKitchen(sem_t* plate)
{
	sem_wait(plate);
}

// İncrease the number of plates on the counter
void supplyPlatetoCounter(sem_t* plate)
{
	sem_post(plate);
}

// Open shared memory
void openExistingObject()
{
	
	fd1 = shm_open("shared1",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd1 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd2 = shm_open("shared2",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd2 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd3 = shm_open("shared3",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd3 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd4 = shm_open("shared4",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd4 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd5 = shm_open("sharedsoup",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd5 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd6 = shm_open("shareddesert",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd6 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd7 = shm_open("sharedmainCourse",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd7 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd8 = shm_open("sharedsoupCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd8 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd9 = shm_open("shareddesertCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd9 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd10 = shm_open("sharedmainCourseCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd10 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd11 = shm_open("cookMutex",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd11 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd12 = shm_open("shared12",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd12 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd13 = shm_open("shared13",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd13 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd14 = shm_open("shared14",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		if (fd14 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd15 = shm_open("shared15",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd15 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd16 = shm_open("shared16",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd16 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd17 = shm_open("shared17",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd17 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd18 = shm_open("shared20",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd18 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}
	fd19 = shm_open("shared22",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd19 == -1)
	{
		perror("Can't open shared memory");
		exit(1);
	}


	emptyKitchen = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
	if (emptyKitchen == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	fullKitchen = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
	if (fullKitchen == MAP_FAILED)
	{

		perror("Failed to mmap");
		exit(1);
	}
	emptyCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);
	if(emptyCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	fullCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd4, 0);
	if (fullCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	soup_atKitchen = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd5, 0);
	if (soup_atKitchen == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	desert_atKitchen = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd6, 0);
	if (desert_atKitchen == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	mainCourse_atKitchen = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd7, 0);
	if (mainCourse_atKitchen == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	soup_atCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd8, 0);
	if (soup_atCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	desert_atCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd9, 0);
	if (desert_atCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	mainCourse_atCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd10, 0);
	if (mainCourse_atCounter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	mutexbetweenCooks = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd11, 0);
	if (mutexbetweenCooks == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	soup_value = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd12, 0);
	if (soup_value == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	desert_value = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd13, 0);
	if (desert_value == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	mainCourse_value = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd14, 0);
	if (mainCourse_value == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	s_counter = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd15, 0);
	if (s_counter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	d_counter = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd16, 0);
	if (d_counter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	m_counter = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd17, 0);
	if (m_counter == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	flag = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd18, 0);
	if (flag == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	_index = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd19, 0);
	if (_index == MAP_FAILED)
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
	if(close(fd6) == -1)
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
	if(close(fd10) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd11) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd12) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd13) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd14) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd15) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd16) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd17) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd18) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd19) == -1)
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

	// Open shared memory
	openExistingObject();
	
	// Save command line arguments
	int order = atoi(argv[1]);
	int l = atoi(argv[2]);
	int m = atoi(argv[3]);

	// Continue until the number of plates cook processes put on the counter is 3*l*m and supplier is finished.
	while(!((*soup_value + *desert_value + *mainCourse_value == 3*l*m) && (*s_counter+*d_counter+*m_counter == 3*l*m)))
	{	
		// Only one cook can work at the same time, because if the lock is not used, decisions can be made wrong.
		sem_wait(mutexbetweenCooks);
		if (*s_counter+*d_counter+*m_counter != 3*l*m)
		{
			
			sem_wait(fullKitchen);	// If kitchen is empty, wait for a plate to get it. 
			sem_wait(emptyCounter);	// If counter is full, wait for a student to take food.

			if (*_index == 0)	// Take soup 
			{
				cook_wait_getDeliveries(order,soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
				takePlateFromKitchen(soup_atKitchen);	// Decrease the number of soups at the kitchen
				cook_goingToCounter(order,"soup",soup_atCounter,mainCourse_atCounter,desert_atCounter);
				supplyPlatetoCounter(soup_atCounter);	// Increase the number of the soups on the counter
				*s_counter = *s_counter + 1;
				*_index = *_index + 1;
				cook_afterDeliveryToCounter(order,"soup",soup_atCounter,mainCourse_atCounter,desert_atCounter);
			}

			else if (*_index == 1) // Take desert
			{
				cook_wait_getDeliveries(order,soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
				takePlateFromKitchen(desert_atKitchen);		// Decrease the number of desert at the kitchen
				cook_goingToCounter(order,"desert",soup_atCounter,mainCourse_atCounter,desert_atCounter);
				supplyPlatetoCounter(desert_atCounter);		// Increase the number of the deserts on the counter
				*d_counter = *d_counter + 1;
				*_index = *_index + 1;
				cook_afterDeliveryToCounter(order,"desert",soup_atCounter,mainCourse_atCounter,desert_atCounter);
			}

			else // Take main course
			{
				cook_wait_getDeliveries(order,soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
				takePlateFromKitchen(mainCourse_atKitchen);		// Decrease the main courses of soups at the kitchen
				cook_goingToCounter(order,"main course",soup_atCounter,mainCourse_atCounter,desert_atCounter);
				supplyPlatetoCounter(mainCourse_atCounter);		// Increase the main courses of the soups on the counter
				*m_counter = *m_counter + 1;
				*_index = *_index + 1;
				cook_afterDeliveryToCounter(order,"main course",soup_atCounter,mainCourse_atCounter,desert_atCounter);
			}

			// If index is three, All three foods are available on the counter at the same time, so a student can take them.
			// Flag semaphore is used between students and cooks. A student should not take one food if the rest of them are not
			// available on the counter. So I use a flag to show if all three are available.
			if (*_index % 3 == 0)
			{
				*_index = 0;
				sem_post(flag);
			}
		
			sem_post(fullCounter);	// Increase the number of plates on the counter
			sem_post(emptyKitchen);	// Increase the space at the kitchen
		}

		sem_post(mutexbetweenCooks);	// Release the lock
	}

	cook_finishingPlacinAllPlates(order,fullKitchen);
	return 0;

}