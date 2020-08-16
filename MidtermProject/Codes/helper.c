#include "helper.h"


int parseCommandLine(int _argc, char* _argv[], int* n, int* m, int* t, int* s, int* l, char* file)
{
	int opt;
	int count = 0; // To check all the options are entered by the user.
	while((opt = getopt(_argc, _argv, ":N:M:T:S:L:F:")) != -1)
	{
		switch(opt)
		{
			case 'N':
			{	
				*n = atoi(optarg);
				++count;
				break;
			}

			case 'M':
			{
				*m = atoi(optarg);
				++count;
				break;
			}

			case 'T':
			{
				*t = atoi(optarg);
				++count;
				break;
			}

			case 'S':
			{	
				*s = atoi(optarg);
				++count;
				break;
			}

			case 'L':
			{
				*l = atoi(optarg);
				++count;
				break;
			}

			case 'F':
			{
				strcpy(file,optarg);
				++count;
				break;
			}

			case ':':
			{
				printf("option needs a value\n");
				exit(1);
			}

			case '?':
			{
				printf("Unknown option %c,You must enter a command line like this : -N 2 -M 10 -T 5 -S 4 -L 13 -F filePath\n", optopt);
				exit(1);
			}
		}
	}

	for ( ; optind < _argc; ++optind)
	{
		printf("You entered extra arguments %s,You must enter a command line like this : -N 2 -M 10 -T 5 -S 4 -L 13 -F filePath\n",_argv[optind]);
		exit(1);
	}

	return count;
}

// This variables can be entered in a coorect way
void checkValidity(int* n, int* m, int* t, int* s, int* l)
{
	if (*n < 3 || *m < 4 || *s < 4 || *l < 3  || *t < 1 || *m <= *n || *m <= *t)
	{	
		printf("All arguments must be positive and follow the rules below:\n");
		printf("M > N > 2 \nS > 3 \nM > T >= 1 \nL>=3\n");
		exit(1);
	}
}



void supplier_doneDelivering()
{	
	write(STDOUT_FILENO,"The supplier finished supplying - GOODBYE!\n",44);
}

void supplier_afterDelivery(char* plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert)
{	
	int soup_value, desert_value, mainCourse_value;
	sem_getvalue(_soup,&soup_value);
	sem_getvalue(_desert,&desert_value);
	sem_getvalue(_mainCourse,&mainCourse_value);
	int total = soup_value + mainCourse_value + desert_value;

	char text[200];
	sprintf(text,"The supplier delivered %s - after delivery kitchen items P: %d, C: %d, D: %d = %d\n",plate,soup_value,mainCourse_value,desert_value,total);
	write(STDOUT_FILENO,text,strlen(text));
}

void supplier_enteringTheKitchen(char* plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert)
{
	int soup_value, desert_value, mainCourse_value;
	sem_getvalue(_soup,&soup_value);
	sem_getvalue(_desert,&desert_value);
	sem_getvalue(_mainCourse,&mainCourse_value);
	int total = soup_value + mainCourse_value + desert_value;

	char text[200];
	sprintf(text,"The supplier is going to the kitchen to deliver %s: kitchen item P: %d, C: %d, D: %d = %d\n",plate,soup_value,mainCourse_value,desert_value,total);
	write(STDOUT_FILENO,text,strlen(text));
}



void cook_finishingPlacinAllPlates(int order, sem_t* items)
{
	int value = sem_getvalue(items,&value);
	char text[200];
	sprintf(text,"Cook %d finished serving - items at kitchen : %d - going home - GOODBYE!!!\n",order,value);
	write(STDOUT_FILENO,text,strlen(text));
}

void cook_afterDeliveryToCounter(int order, char* _plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert)
{
	int soup_value, desert_value, mainCourse_value;
	sem_getvalue(_soup,&soup_value);
	sem_getvalue(_desert,&desert_value);
	sem_getvalue(_mainCourse,&mainCourse_value);
	int total = soup_value + mainCourse_value + desert_value;
	char text[200];
	sprintf(text,"Cook %d placed %s on the counter - counter items P: %d, C: %d, D: %d = %d\n",order,_plate,soup_value,mainCourse_value, desert_value, total);
	write(STDOUT_FILENO,text,strlen(text));
}

void cook_goingToCounter(int order,char* plate, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert)
{
	int soup_value, desert_value, mainCourse_value;
	sem_getvalue(_soup,&soup_value);
	sem_getvalue(_desert,&desert_value);
	sem_getvalue(_mainCourse,&mainCourse_value);
	int total = soup_value + mainCourse_value + desert_value;

	char text[200];
	sprintf(text,"Cook %d going to the counter to deliver %s - counter items P: %d, C: %d, D: %d = %d\n",order,plate,soup_value,mainCourse_value, desert_value, total);
	write(STDOUT_FILENO,text,strlen(text));
}

void cook_wait_getDeliveries(int order, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert)
{
	int soup_value, desert_value, mainCourse_value;
	sem_getvalue(_soup,&soup_value);
	sem_getvalue(_desert,&desert_value);
	sem_getvalue(_mainCourse,&mainCourse_value);
	int total = soup_value + mainCourse_value + desert_value;

	char text[200];
	sprintf(text,"Cook %d is going to the kitchen to wait for/get a plate - kitchen items P: %d, C: %d, D: %d = %d\n",order,soup_value,mainCourse_value, desert_value, total);
	write(STDOUT_FILENO,text,strlen(text));
}



void student_finishingEating(int order, int L)
{
	char text[200];
	sprintf(text,"Student %d is done eating L=%d times - going home - GOODBYE\n",order,L);
	write(STDOUT_FILENO,text,strlen(text));
}

void student_goingAgainToCounter(int order, int no, int L, sem_t* _table)
{
	char text[200];
	int table;
	sem_getvalue(_table,&table);
	sprintf(text,"Student %d left table %d to eat again (round %d) - empty tables: %d\n",order,no,L,table);
	write(STDOUT_FILENO,text,strlen(text));
}

void student_sittingToEat(int order, int no, int L, sem_t* _table)
{
	char text[200];
	int table;
	sem_getvalue(_table,&table);
	sprintf(text,"Student %d sat at table %d to eat (round %d) - empty tables: %d\n",order,no,L,table);
	write(STDOUT_FILENO,text,strlen(text));
}

void student_wait_getTable(int order, int L, sem_t* _table)
{
	char text[200];
	int table;
	sem_getvalue(_table,&table);
	sprintf(text,"Student %d got food and is going to get a table (round %d) - # of empty tables: %d\n",order,L,table);
	write(STDOUT_FILENO,text,strlen(text));
}

void student_waitingForFood(int order, int L, int* numOfStudents, sem_t* _soup, sem_t* _mainCourse, sem_t* _desert)
{
	char text[200];
	int soup_value, desert_value, mainCourse_value;
	sem_getvalue(_soup,&soup_value);
	sem_getvalue(_desert,&desert_value);
	sem_getvalue(_mainCourse,&mainCourse_value);
	sprintf(text,"Student %d is going to counter (round %d) - # of students at counter: %d and counter items P:%d, C:%d, D:%d\n",order,L,*numOfStudents,soup_value,mainCourse_value,desert_value);
	write(STDOUT_FILENO,text,strlen(text));
}


