#ifndef HELPER_H_
#define HELPER_H_ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>



// to parse command line arguments
int parseCommandLine(int _argc, char* _argv[], int* n, int* m, int* t, int* s, int* l,char* file);

// to check if values meets the conditions
void checkValidity(int* n, int* m, int* t, int* s, int* l); 


// To print messages about state of the supplier proces
void supplier_doneDelivering();
void supplier_afterDelivery(char* plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert);
void supplier_enteringTheKitchen(char* plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert);


// To print messages about state of the cook process
void cook_finishingPlacinAllPlates(int order, sem_t* items);
void cook_afterDeliveryToCounter(int order, char* _plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert);
void cook_goingToCounter(int order, char* plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert);
void cook_wait_getDeliveries(int order, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert);


// To print messages about state of the student process
void student_finishingEating(int order, int L);
void student_goingAgainToCounter(int order, int no, int L, sem_t* _table);
void student_sittingToEat(int order, int no, int L, sem_t* _table);
void student_wait_getTable(int order, int L, sem_t* _table);
void student_waitingForFood(int order, int L, int* numOfStudents, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert);



#endif