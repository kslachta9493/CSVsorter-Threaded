
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sorter.h"
#include "mergesort.c"
#include "builder.h"
#include <dirent.h>


SORTERARGS *sortingparam;
CSVPARAMS *csvparams;
/*
	TO-DO:	
		walk entire directory					NOT-DONE
		fork for every .csv found				NOT-DONE
		open on filename					NOT-DONE
		check if valid csv					NOT-DONE
		save every pid generated				NOT-DONE
		output sorted file to filename-sorted-.csv		NOT-DONE
		add more functions to view code better			NOT-DONE
		add more flag for command line input			NOT-DONE
*/
			

int loadAndSort ()
{
	SORTERARGS *sortingargs = (SORTERARGS *) sortingparam;
	//iterators
	int i = 0;
	int q = 0;
	int j = 0;

	//Data trackers
	int totalpids = 0;
	int totalcsv = 0;
	int initpid = getpid();


	//filechecks
	int linesize = 300;
	int stringsize = 50000;

	//file buffer
	char *holder;
	char *structinput = (char*) malloc (linesize*sizeof(char));
	char *tmp;

	//function declaration
	char* buildBuffer(int, char*);
	void printMetadata(int, int, int[]);
	int checkType(char*);
	int validateinput(char*);
	int checkInputString(char*, CSV input[], int argc);
	void populateDIR(char *name);

	char* userinputdirectory = sortingparam->inputdirectory;
	char* name = sortingparam->filename;
	char* colname = sortingparam->colname;
	int coltosorton = sortingparam->coltosorton;
	int argc = sortingparam->argc;
	

	/*

	/*
		Build massive buffer and record rows and columns for later
		Counts amount of commas in first line to determine column amount
		while ignoring commas inside of quotes
	*/
	char *path1 = (char*) malloc ((strlen(userinputdirectory) + strlen(name) + 2) * sizeof(char));
	strcpy(path1, userinputdirectory);
	strcat(path1, "/");
	strcat(path1, name);
	printf("SORTER%s %s %s\n", userinputdirectory, name, path1);


	holder = buildBuffer(stringsize, path1);
	free(path1);
	/*
		check for valid user input
		update this to include new flags
	*/
	if (validateinput(holder) == 1)
	{
		free(structinput);
		free(holder);
		exit(-1);
	}

	/*
		Define and initialize all my dynamic variables to hold the data
		to be sorted and the correct way to sort each type
		CSV input is an array of structs that holds every token
		from the input csv file
		the int type array holds a value 0,1,2 for the type of value it holds such that
		0 = string
		1 = int
		2 = float
	*/
	i = 0;
	int rows = csvparams->rows;
	int cols = csvparams->cols;
	CSV input[rows];
	int type[cols];

	for (i = 0; i < cols; i++)
	{
		type[i] = 0;
	}
	for (i = 0; i < rows; i++)
	{
		input[i].filedata = (char**) malloc(sizeof(char*) * cols);
	}
	/*
		Parsing the entire character buffer we made earlier
		Line = currect line of the csv = current input[i] such that the input array resembles the csv input
		structline = the secondary array inside of the struct input creating a 2d "string" array
		ignore is a check for quotes and skips the split if a comma is detected inside a quote
		removewhite is a check to strip leading and trailing whitespace
	*/
	buildCSV(holder, input, structinput, linesize, rows, cols);

	//if user input sorter string matches, then sort on that column and then output the data to stdout
	if (coltosorton == -1)
	{
		printf("\nInvalid sorting string. Be sure it matches a column header! CASE SENSITIVE!\n");
		exit(0);
	} else {
		coltosorton = checkInputString(colname, input, argc);
	}
	if (coltosorton == -1 )
	{
		printf("\nInvalid sorting string. Be sure it matches a column header! CASE SENSITIVE!\n");
	} else
	{
		//get first line of data and assign a value to each one, 0 for string, 1 for int, 2 for float
		for (i = 0; i < cols; i++)
		{
			type[i] = checktype(input[1].filedata[i]);
		}
		//Simply output the sorted data in input to stdout

		
		char *outpath = (char*) malloc ((strlen(name) + strlen(input[0].filedata[coltosorton]) + 11) * sizeof(char));
		name[strlen(name) - 4] = 0;
		strcpy(outpath, ".");
		strcat(outpath, "/");
		strcat(outpath, name);
		strcat(outpath, "-sorted-");
		strcat(outpath, input[0].filedata[coltosorton]);
		strcat(outpath, ".csv");
		
		mergeSort(input, 1, rows - 1, coltosorton, type[coltosorton]);

		//global BST
		//LOCK when inputting into BST

		//printSorted(input, rows, cols, outpath);
		//put in BST
	}
	
	//Give back the memory so the OS doesn't hate me
	i = 0;
	j = 0;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			free(input[i].filedata[j]);
		}
	}
	for (i = 0; i < rows; i++)
	{
		free(input[i].filedata);
	}
	free(holder);
	free(structinput);
	return 1;

}

/*
	FUNCTION NAME: checktype	

	PURPOSE:
		Return a value to define the type of data passed to it. Determines whether the char* given to the function is a string,
		float or int value.
	
	PARAMETERS:
	name		type			value/reference		description
	----------------------------------------------------------------------------
	test		char*			value			char* to be defined as int string or float
	
	RETURN VALUE: int

	CALLED FROM: main

	METHOD: The following is pseudocode describing how the function works.
		1. Check if the char* has any letters in it
			1a. if it contains a letter it must be a string and return 0;
		2. If no letters are found it must be an int or a float
			2a. if it contains a decimal it must be a float
			2b. if not it must be an int
		return  0 for string
			1 for int
			2 for float
	
*/
int checktype (char *test)
{
	int check = 0;
	int i = 0;
	for (i = 0; i < strlen(test); i++)
	{
		if (isalpha(test[i]))
		{
			//if any letter is found, define it as a string
			return 0;
		} else {
			if (test[i] == '.')
			{
				//if a decimal is found mark it as a possible float
				check = 1;
			}
		}
	}

	/*
		Should the method reach this if statement, no letters were found in the string
		therefor it must be an int or a float, if a decimal was found earlier
		it must be a float, if not it must be an int
	*/

	if (check == 1) 
	{
		return 2;
	} else {
		return 1;
	}
	return 0;
}
int checkInputString (char *colname, CSV input[], int argc)
{
	//Check if user input sorting string matchs any column from the given csv
	int i = 0;
	int j = 0;
	int acceptedargs = -1;
	for (i = 0; i < csvparams->cols; i++)
	{
		//printf("%s : %s\n", colname, input[0].filedata[j]);
		if (strcmp(colname, input[0].filedata[i]) == 0)
		{
			acceptedargs = i;
		}
	}
	return acceptedargs;
}

/*
	FUNCTION NAME: validateinput	

	PURPOSE:
		Program rules dictate we must be given 2 program flags to run with
		-c must be present followed by a SORTSTRING. This function checks if
		user given data matches the program requirements.
	
	PARAMETERS:
	name		type			value/reference		description
	----------------------------------------------------------------------------
	argc		int			value			number to be compared to expected amount of arguments
	argv		char**			value			values the user gave to be compared to expected arguments

	RETURN VALUE: int

	CALLED FROM: main

	METHOD: The following is pseudocode describing how the function works.
		1. Compare the passed value to a static value defined
		2. if greater than the static value output too many inputs to user!
		3. if lesser than the static value output too many inputs to user!
		4. if equal to the static value check if the first argument is equal
			to the static flag -c	
	
*/
int validateinput (char* holder)
{
	if (holder[0] == '\0') {
		printf("FILE NOT FOUND! please be sure you typed the file name correctly\n");
		return 1;
	}
	return 0;
}
char * buildBuffer(int stringsize, char* userinputdirectory)
{
	char *holdera = (char*) malloc (stringsize * sizeof(char));
	int i = 0;
	int firstline = 0;
	int ignore = 0;
	char *tmp;

	int cols = 0;
	int rows = 0;
	for(i = 0; i < stringsize; i++)
	{
		holdera[i] = '\0';
	}
	i = 0;

	FILE *fp;
	fp = fopen(userinputdirectory, "r");
	//printf("OPEN: %s\n", userinputdirectory);
	if (fp == NULL)
		printf("FAILED TO OPEN %s\n", userinputdirectory);
	while (i < stringsize)
	{
		int test = fscanf(fp, "%c", &holdera[i]);
		if (test == EOF) {
			break;
		}
		if (holdera[i] == '\n')
		{
			firstline = 1;
			rows++;
		}
		if (holdera[i] == '"' && ignore == 0)
		{
			ignore = 1;
		} else if (holdera[i] == '"' && ignore == 1)
		{
			ignore = 0;
		}
		if (holdera[i] == ',' && ignore == 0 && firstline == 0)
		{
			cols++;
		}
		i++;
		if (i == stringsize) {
			int large = stringsize * 2;
			tmp = (char*) realloc (holdera, large*sizeof(char));
			if (tmp == NULL)
			{
				printf("Malloc FAILED!\n");
				exit(-1);
			} else 
			{
				holdera = tmp;
				stringsize = large;
			}
			int reset = 0;
			for(reset = stringsize / 2; reset < stringsize; reset++)
			{
				holdera[i] = '\0';
			}
		}
	}
	cols++;
	if (fp != NULL)
		fclose(fp);

	csvparams->cols = cols;
	csvparams->rows = rows;
	return holdera;
}

void test (void* input) {
	SORTERARGS *holder = (SORTERARGS *) input;
	printf("VALUE %s\n", holder->filename);
}
void initsorting(void *inputstruct) {
	sortingparam = (SORTERARGS *) malloc(sizeof(SORTERARGS));
	csvparams = (CSVPARAMS *) malloc(sizeof(CSVPARAMS));

	SORTERARGS *sortingargs = (SORTERARGS *) inputstruct;

	sortingparam->colname = sortingargs->colname;
	sortingparam->argc = sortingargs->argc;
	sortingparam->coltosorton = sortingargs->coltosorton;
	sortingparam->filename = sortingargs->filename;
	sortingparam->inputdirectory = sortingargs->inputdirectory;

}
