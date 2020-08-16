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
				write(STDOUT_FILENO,"option needs a value\n",22);
				exit(1);
			}

			case '?':
			{
				char text[200];
				sprintf(text,"Unknown option %c,You must enter a command line like this : -i filePath\n",optopt);
				write(STDOUT_FILENO,text,strlen(text));
				exit(1);
			}
		}
	}

	for ( ; optind < _argc; ++optind)
	{
		char text[200];
		sprintf(text,"You entered extra arguments %s,You must enter a command line like this : -i filePath\n",_argv[optind]);
		write(STDOUT_FILENO,text,strlen(text));
		exit(1);
	}


	return count;
}



float findChebyshevDistance(float client_x, float client_y, float florist_x, float florist_y)
{
	int x = abs(client_x - florist_x);
	int y = abs(client_y - florist_y);

	if (x > y)
		return x;
	return y;
}


void florist_hasDelivered(char* _name, char* _flower, char* clientNum, float _time)
{
	char text[100];
	sprintf(text,"Florist %s has delivered a %s to %s in %1.fms\n",_name,_flower,clientNum,_time);
	write(STDOUT_FILENO,text,strlen(text));
}

void florist_Closing(char* _name)
{
	char text[100];
	sprintf(text,"%s closing shop\n",_name); 
	write(STDOUT_FILENO,text,strlen(text));
}

void printStatics(struct Statistic** tmp, struct florist* _florist, int count)
{

	char text[500];
	write(STDOUT_FILENO,"Sale statistics for today:\n",28);
	sprintf(text,"%-20s %-20s %-20s\n", "Florist","# of sales","Total time");
	write(STDOUT_FILENO,"-------------------------------------------------------\n",57);
	write(STDOUT_FILENO,text,strlen(text));
	write(STDOUT_FILENO,"-------------------------------------------------------\n",57);
	for (int i = 0; i < count; ++i)
	{	
		int sale = tmp[i]->totalOrder;
		float time = tmp[i]->total_time;
		sprintf(text,"%-20s %-20d %-20f\n\n",_florist[i].name,sale,time);
		write(STDOUT_FILENO,text,strlen(text));
	}
}



