
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sorter.h"
#include "builder.h"
#include <dirent.h>


int threadcount = 0;
int totalcsv = 0;
int threadmax = 5000;
SORTERARGS* argstruct;

pthread_t *threads;
pthread_mutex_t lock;
char *userinputdirectory = NULL;
char *useroutputdirectory = NULL;

//Function Declarations
void *dirsort(void*);
void *sort(void*);
void doublethread();


int main (int argc, char* argv[])
{

	int i = 0;
	int parentpid = getpid();
	int freeinput = 0;
	int freeoutput = 0;
	argstruct = (SORTERARGS *) malloc (sizeof(SORTERARGS) * threadmax);

	for (i = 0; i < threadmax; i++)
	{
		(argstruct + i)->argc = argc;
	}

	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("MUTEX INIT FAILED");
		exit(0);
	}
	printf("Initial PID: %d\n", parentpid);
	printf("Child TIDS: ");
	fflush(stdout);



	argstruct->coltosorton = validflags(argc, argv);

	if (userinputdirectory == NULL)
	{
		userinputdirectory = (char*) malloc (2*sizeof(char));
		userinputdirectory[0] = '.';
		userinputdirectory[1] = '\0';
		for (i = 0; i < threadmax; i++)
		{
			(argstruct + i)->inputdirectory = strdup(userinputdirectory);
		}

		free(userinputdirectory);
	} else {
		FILE* fp;
		fp = fopen(userinputdirectory, "r");
	
		if (fp == NULL)
		{
			printf("\nInvalid input directory %s\n", userinputdirectory);
			exit(-1);
		}
		close(fp);
	}
	if (useroutputdirectory == NULL)
	{
		
		useroutputdirectory = (char*) malloc (2*sizeof(char));
		useroutputdirectory[0] = '.';
		useroutputdirectory[1] = '\0';
		
		//useroutputdirectory = strdup(userinputdirectory);
		freeoutput = 1;
	} else
	{
		FILE* fp;
		fp = fopen(useroutputdirectory, "r");
	
		if (fp == NULL)
		{
			printf("\nInvalid output directory %s\n", useroutputdirectory);
			exit(-1);
		}
		close(fp);
	}

	threads = (pthread_t *) malloc (threadmax * sizeof(pthread_t));
	
	populateDIR(userinputdirectory);

	//wait for all threads

	for (i = 0; i < threadcount; i++)
	{
		pthread_join(threads[i], NULL);
		if (i == 0)
			printf(" %d", threads[i]);
		else
			printf(", %d", threads[i]);
	}

	printf("\nTotal Threads: %d\n", threadcount);
	
	if (freeinput)
		free(userinputdirectory);
	
	if (freeoutput)
		free(useroutputdirectory);

	return 0;
}

int validflags(int argc, char* argv[])
{
	int i = 0;
	int acceptedargs = -1;
	int j = 0;
	for (i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-c") == 0)
		{
			if (argv[i + 1] != NULL)
			{
				acceptedargs = 0;
				//argstruct->colname = strdup(argv[i + 1]);
				for (j = 0; j < threadmax; j++)
				{
					(argstruct + j)->colname = strdup(argv[i + 1]);
				}
			}
		}
		if (strcmp(argv[i], "-d") == 0)
		{
			userinputdirectory = strdup(argv[i + 1]);
		}
		if (strcmp(argv[i], "-o") == 0)
		{
			useroutputdirectory = strdup(argv[i + 1]);
		}
	}
	return acceptedargs;
}

int populateDIR(char *name)
{
    	DIR *dir;
    	struct dirent *entry;

   	if (!(dir = opendir(name)))
        	return 0;

	while ((entry = readdir(dir)) != NULL) 
	{
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
			} else
			{
		    		char *path = (char*) malloc ((strlen(name) + strlen(entry->d_name) + 2) * sizeof(char));
				strcpy(path, name);
				strcat(path, "/");
				strcat(path, entry->d_name);

				pthread_mutex_lock(&lock);
				if (threadcount == threadmax)
				{
					doublethread();
				}
				pthread_create(&threads[threadcount], NULL, dirsort, (void *) path);
				threadcount++;
				pthread_mutex_unlock(&lock);

			}
        	} else if (entry->d_type == DT_REG) 
		{
			int length = strlen(entry->d_name);
			if (entry->d_name[length - 1] == 'v' && entry->d_name[length - 2] == 's' && entry->d_name[length - 3] == 'c' && entry->d_name[length - 4] == '.')
			{
				if (!strstr(entry->d_name, "-sorted-"))
				{
					pthread_mutex_lock(&lock);

					if (threadcount == threadmax)
					{
						doublethread();
					}

					(argstruct + totalcsv)->filename = strdup(entry->d_name);
					(argstruct + totalcsv)->inputdirectory = strdup(name);
					pthread_create(&threads[threadcount], NULL, sort, (void *) (argstruct + totalcsv));
					totalcsv++;
					threadcount++;

					pthread_mutex_unlock(&lock);
				}
			}
       		}
    	}
	closedir(dir);
	return 0;
}
void *dirsort(void *args)
{
	char *dirname = (char *) args;
	populateDIR(dirname);
	pthread_exit(0);
}
void *sort(void *args)
{
	initsorting((void *) args);
	loadAndSort();
	pthread_exit(0);
}
void doublethread()
{
	int large = threadmax * 2;
	pthread_t *tmp = (pthread_t *) realloc (threads, large*sizeof(pthread_t));
	if (tmp == NULL)
	{
		printf("Malloc FAILED!\n");
		exit(-1);
	} else 
	{
		threads = tmp;
		threadmax = large;
	}
}

