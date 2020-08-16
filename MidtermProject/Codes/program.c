#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
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
sem_t* table;
sem_t* flag;
int* numberOfStudents;
int* soup_value;
int* desert_value;
int* mainCourse_value;
int* s_counter;
int* d_counter;
int* m_counter;
int* _index;

char* filename;
char* buf;
int fd1, fd2, fd3, fd4 ,fd5, fd6, fd7, fd8, fd9, fd10, fd11, fd12, fd13, fd14, fd15, fd16, fd17, fd19, fd20, fd21, fd22;


void exitGracefully()
{
	free(filename);
	free(buf);
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
	close(fd19);
	close(fd20);
	close(fd21);
	close(fd22);
	sem_destroy(fullKitchen);
	sem_destroy(emptyKitchen);
	sem_destroy(fullCounter);
	sem_destroy(emptyCounter);
	sem_destroy(soup_atKitchen);
	sem_destroy(desert_atKitchen);
	sem_destroy(mainCourse_atKitchen);
	sem_destroy(soup_atCounter);
	sem_destroy(desert_atCounter);
	sem_destroy(mainCourse_atCounter);
	sem_destroy(mutexbetweenCooks);
	sem_destroy(flag);
	munmap(emptyKitchen,sizeof(sem_t));
	munmap(fullKitchen,sizeof(sem_t));
	munmap(emptyCounter,sizeof(sem_t));
	munmap(fullCounter,sizeof(sem_t));
	munmap(soup_atKitchen,sizeof(sem_t));
	munmap(desert_atKitchen,sizeof(sem_t));
	munmap(mainCourse_atKitchen,sizeof(sem_t));
	munmap(soup_atCounter,sizeof(sem_t));
	munmap(desert_atCounter,sizeof(sem_t));
	munmap(mainCourse_atCounter,sizeof(sem_t));
	munmap(mutexbetweenCooks,sizeof(sem_t));
	munmap(flag,sizeof(sem_t));
	munmap(table,sizeof(sem_t));
	munmap(numberOfStudents,sizeof(int));
	munmap(soup_value,sizeof(int));
	munmap(desert_value,sizeof(int));
	munmap(mainCourse_value,sizeof(int));
	munmap(s_counter,sizeof(int));
	munmap(d_counter,sizeof(int));
	munmap(m_counter,sizeof(int));
	munmap(index,sizeof(int));
	shm_unlink("shared1");
	shm_unlink("shared2");
	shm_unlink("shared3");
	shm_unlink("shared4");
	shm_unlink("sharedsoup");
	shm_unlink("shareddesert");
	shm_unlink("sharedmainCourse");
	shm_unlink("sharedsoupCounter");
	shm_unlink("shareddesertCounter");
	shm_unlink("sharedmainCourseCounter");
	shm_unlink("cookMutex");
	shm_unlink("shared12");
	shm_unlink("shared13");
	shm_unlink("shared14");
	shm_unlink("shared15");
	shm_unlink("shared16");
	shm_unlink("shared17");
	shm_unlink("shared19");
	shm_unlink("shared20");
	shm_unlink("shared21");
	shm_unlink("shared22");
	exit(1);
}


// To Catch SIGINT signal
void handler(int signo)
{	
	while(waitpid(-1,NULL,WNOHANG) > 0);
	exitGracefully();
}

// İncrease the value of the semaphore
void supplyPlate(sem_t* plate)
{
	sem_post(plate);
}


// To create shared memory
void createSharedMemory()
{
	
	fd1 = shm_open("shared1",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd1 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd2 = shm_open("shared2",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd2 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd3 = shm_open("shared3",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd3 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd4 = shm_open("shared4",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd4 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd5 = shm_open("sharedsoup",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd5 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd6 = shm_open("shareddesert",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd6 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd7 = shm_open("sharedmainCourse",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd7 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd8 = shm_open("sharedsoupCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd8 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd9 = shm_open("shareddesertCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd9 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd10 = shm_open("sharedmainCourseCounter",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd10 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd11 = shm_open("cookMutex",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd11 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd12 = shm_open("shared12",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd12 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd13 = shm_open("shared13",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd13 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd14 = shm_open("shared14",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd14 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd15 = shm_open("shared15",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd15 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd16 = shm_open("shared16",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd16 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd17 = shm_open("shared17",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd17 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd19 = shm_open("shared19",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd19 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd20 = shm_open("shared20",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd20 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd21 = shm_open("shared21",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd21 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}
	fd22 = shm_open("shared22",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd22 == -1)
	{
		perror("Can't create shared memory");
		exit(1);
	}


	// Set their sizes 
	if (ftruncate(fd1,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd2,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd3,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd4,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd5,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd6,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd7,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd8,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd9,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd10,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd11,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd12,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd13,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd14,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd15,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd16,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd17,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd19,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd20,sizeof(sem_t)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd21,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
		exit(1);
	}
	if (ftruncate(fd22,sizeof(int)) == -1)
	{
		perror("Can't resize shared memory");
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
	emptyCounter = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0	);
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
	table = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd19, 0);
	if (table == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	flag = mmap(NULL,sizeof(sem_t),PROT_READ | PROT_WRITE, MAP_SHARED, fd20, 0);
	if (flag == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	numberOfStudents = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd21, 0);
	if (numberOfStudents == MAP_FAILED)
	{
		perror("Failed to mmap");
		exit(1);
	}
	_index = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd22, 0);
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
	if(close(fd19) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd20) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd21) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
	if(close(fd22) == -1)
	{
		perror("Failed to close descriptor of the shared memory");
		exit(1);
	}
}


// To initialize semaphore
void initializeSemaphores(int _k, int _s, int _t)
{
	if (sem_init(emptyKitchen,1,_k) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(fullKitchen,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(emptyCounter,1,_s) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(fullCounter,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(soup_atKitchen,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(desert_atKitchen,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(mainCourse_atKitchen,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(soup_atCounter,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(desert_atCounter,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(mainCourse_atCounter,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(mutexbetweenCooks,1,1) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(table,1,_t) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	} 
	if(sem_init(flag,1,0) == -1)
	{
		perror("Failed to initialize semaphore");
		exit(1);
	}

}

// To destroy semaphores
void destroySemaphores()
{
	if(sem_destroy(fullKitchen) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(emptyKitchen) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(fullCounter) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(emptyCounter) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(soup_atKitchen) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(desert_atKitchen) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(mainCourse_atKitchen) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(soup_atCounter) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(desert_atCounter) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(mainCourse_atCounter) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(mutexbetweenCooks) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(table) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
	if(sem_destroy(flag) == -1)
	{
		perror("Failed to destroy semaphores");
		exit(1);
	}
}

void clearSharedMemory()
{
	if (munmap(emptyKitchen,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(fullKitchen,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(emptyCounter,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(fullCounter,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(soup_atKitchen,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(desert_atKitchen,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(mainCourse_atKitchen,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(soup_atCounter,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(desert_atCounter,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(mainCourse_atCounter,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(mutexbetweenCooks,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(flag,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(table,sizeof(sem_t)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(numberOfStudents,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(soup_value,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	} 
	if (munmap(desert_value,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(mainCourse_value,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(s_counter,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(d_counter,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(m_counter,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}
	if (munmap(_index,sizeof(int)) == -1)
	{
		perror("Error is occured while using munmap");
		exit(1);
	}


	if (shm_unlink("shared1") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared2") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared3") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared4") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("sharedsoup") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shareddesert") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("sharedmainCourse") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("sharedsoupCounter") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shareddesertCounter") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("sharedmainCourseCounter") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("cookMutex") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared12") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared13") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared14") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared15") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared16") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared17") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared19") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared20") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared21") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
	if (shm_unlink("shared22") == -1)
	{
		perror("Error is occured while using shm_unlink");
		exit(1);
	}
}


void createCookProcess(int _i, int _l, int _m)
{
	char order[20];
	char l[20];
	char m[20];
	sprintf(order,"%d",_i);
	sprintf(l,"%d",_l);
	sprintf(m,"%d",_m);
	char* const list[] = {"cook",order,l,m,(char*)0};
	pid_t id;
	fflush(stdout);
	id = fork();
	if (id == 0)
	{	
		execve("cook",list,NULL);
	}

	if(id == -1)
	{
		printf("Failed to create cook processes\n");
		exitGracefully();
	}
}

void createStudentProcess(int _i, int _l, int _t)
{
	char order[20];
	char l[20];
	char t[20];
	sprintf(order,"%d",_i);
	sprintf(l,"%d",_l);
	sprintf(t,"%d",_t);
	char* const list[] = {"student",order,l,t,(char*)0};
	pid_t id;
	fflush(stdout);
	id = fork();
	if (id == 0)
	{	
		execve("student",list,NULL);
	}

	if(id == -1)
	{
		printf("Failed to create student processes\n");
		exitGracefully();
	}
}



int main(int argc, char *argv[])
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
	
	// n is number of cooks
	// m is number of students
	// t is number of tables
	// s is number of counter
	// l is number of total time for eating
	// k is number of kitchen
	// count is the number of command line arguments
	int n, m, t, s, l, k, count;
	filename = (char*) malloc(sizeof(char)*100);
	count = parseCommandLine(argc, argv, &n,&m,&t,&s,&l,filename);
	if (count != 6)
	{
		printf("You entered missing command lines, You must enter a command line like this : -N 2 -M 10 -T 5 -S 4 -L 13 -F filePath\n");
		free(filename);
		exit(1);
	}

	checkValidity(&n,&m,&t,&s,&l);	// if arguments are invalid exit.

	buf = (char*) malloc(sizeof(char)*(3*l*m));
	int i, fd, c = 0, p = 0, d = 0, size = 0;
	k = 2*l*m + 1;

	srand(time(NULL));

	// Create shared memory to communicate between processes and initialize semaphores.
	createSharedMemory();
	initializeSemaphores(k,s,t);


	// soup_value, desert_value and mainCourse_value indicate the number of plates that supplier puts in the kitchen. 
	*soup_value = 0;
	*desert_value = 0;
	*mainCourse_value = 0;
	// s_counter, d_counter and m_counter indicate the number of plates that cooks put in the counter.
	*s_counter = 0;
	*d_counter = 0;
	*m_counter = 0;
	// It indicates how many students are waiting to get food.
	*numberOfStudents = 0;
	// This variable is needed for the algorithm of the cook process. Cooks put the plates in order to prevent deadlocks.
	*_index = 0;

	// Open the input file in readonly mode
	fd = open(filename,O_RDONLY);		
	if (fd == -1)
	{
		perror("Failed to open input file");
		exitGracefully();
	}

	int sizeTest = lseek(fd,0,SEEK_END);
	if (sizeTest != 3*l*m)
	{
		printf("File does not contain exactly 3*l*m characters\n");
		close(fd);
		exitGracefully();
	}

	lseek(fd,0,SEEK_SET);

	// Read the file 
	while (size != 3*l*m)					
	{
		size = read(fd,buf,3*l*m);
		if (size == -1)
		{
			perror("Failed to read input file");
			close(fd);
			exitGracefully();
		}

		
		// Check if every character is correct. If the character that is read from file is not P, D or M exit
		if (size == 3*l*m)	
		{	
			for (i = 0; i < size; ++i)
			{
				if (buf[i] == 'P')
				{
					++p;
				}
				if (buf[i] == 'D')
				{
					++d;
				}
				if (buf[i] == 'C')
				{
					++c;
				}
			}
		}

		if (p != l*m || d != l*m || c != l*m)
		{	
			printf("The number of plates in that file are invalid!! Please enter l*m soups, l*m deserts and l*m main courses\n");
			close(fd);
			exitGracefully();
		}
	}

	if (close(fd) == -1)
	{
		perror("Failed to close the descriptor of the input file");
		exitGracefully();
	}

	// Create n cook processes
	for (i = 0; i < n; ++i)
	{
		createCookProcess(i,l,m);
	}

	// Create m students
	for (int i = 0; i < m; ++i)
	{
		createStudentProcess(i,l,t);
	}
		

	// Until end of the file 
	i = 0;
	while(i != size)
	{	// Decrease space at the kitchen
		sem_wait(emptyKitchen);	// If kitchen is not empty, wait for it to supply food

		// If current food is soup
		if (buf[i] == 'P')
		{	
			supplier_enteringTheKitchen("soup",soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
			supplyPlate(soup_atKitchen);	// post the semaphore, increase the number of soups on the counter
			*soup_value = *soup_value + 1;
			supplier_afterDelivery("soup",soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
		}

		// If current food is desert
		else if(buf[i] == 'D')
		{	
			supplier_enteringTheKitchen("desert",soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
			supplyPlate(desert_atKitchen);	// post the semaphore, increase the number of deserts on the counter
			*desert_value = *desert_value + 1;
			supplier_afterDelivery("desert",soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
		}

		// If current food is main course
		else if(buf[i] == 'C') 
		{	
			supplier_enteringTheKitchen("mainCourse",soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
			supplyPlate(mainCourse_atKitchen);	// post the semaphore, increase the number of main courses on the counter
			*mainCourse_value = *mainCourse_value + 1;;
			supplier_afterDelivery("mainCourse",soup_atKitchen,mainCourse_atKitchen,desert_atKitchen);
		}

		else
		{
			printf("Invalid character\n");
			exitGracefully();
		}

		++ i;
		sem_post(fullKitchen);	// Increase the number of plates in the kitchen 
	}

	supplier_doneDelivering();

	// Wait for all children to be dead
	while(wait(NULL) > 0);



	//free resources
	free(filename);
	free(buf);
	destroySemaphores();
	clearSharedMemory();

	return 0;

}