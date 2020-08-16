#include "helper.h"


int parseCommandLine(int _argc, char* _argv[], char* file)
{
	int opt;
	int count = 0; // To check all the options are entered by the user.
	while((opt = getopt(_argc, _argv, ":i:")) != -1)
	{
		switch(opt)
		{

			case 'i':
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
				printf("Unknown option %c,You must enter a command line like this : -i filePath\n", optopt);
				exit(1);
			}
		}
	}

	for ( ; optind < _argc; ++optind)
	{
		printf("You entered extra arguments %s,You must enter a command line like this : -i filePath\n",_argv[optind]);
		exit(1);
	}

	return count;
}





void wholesaler_isWaiting()
{
	write(STDOUT_FILENO,"The wholesaler is waiting for the dessert\n",43);
}

void wholesaler_Delivering(char* ing1, char* ing2)
{
	char text[100];
	sprintf(text,"The wholesaler delivers %s and %s\n",ing1,ing2);
	write(STDOUT_FILENO,text,strlen(text));
}

void wholesaler_Obtained()
{
	write(STDOUT_FILENO,"The wholesaler has obtained the dessert and left to sell it\n",61);
}




void cook_isWaiting(int order, char* ing1, char* ing2)
{
	char text[100];
	sprintf(text,"Chef%d is waiting for %s and %s\n",order,ing1,ing2);
	write(STDOUT_FILENO,text,strlen(text));
}

void cook_hasTaken(int order, char* ing)
{
	char text[100];
	sprintf(text,"Chef%d has taken the %s\n",order,ing);
	write(STDOUT_FILENO,text,strlen(text));
}

void cook_isPreparing(int order)
{
	char text[100];
	sprintf(text,"Chef%d is preparing the dessert\n",order);
	write(STDOUT_FILENO,text,strlen(text));
}

void cook_hasDelivered(int order)
{
	char text[100];
	sprintf(text,"Chef%d has delivered the dessert to the wholesaler\n",order);
	write(STDOUT_FILENO,text,strlen(text));
}
