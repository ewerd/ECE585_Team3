//
//  main.c
//  ECE_585_Parse
//
//  Created by Drew Seidel, Braden Harwood, Michael Weston and Stephen Short on 10/26/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "preliminary_parser.h"
#define SIZEARRAYS 100


/*
Preliminary parser for file type of ECE 585 project
 
Pros:
-readfile function parses time into int
-readfile function deciphers instruction command
-readfile function converts hex string to hex value
-does what it needs to for Friday's 10/29 checkpoint
-^could be desirable and could not be desirable
 
 Cons:
 -I ask about printing before the filename which makes no sense
 -Memory allocation = basic and poor
 -One long function for readfile, and a short one for concanate
 -Make seperate functions for data conversions in readfile perhaps
 -Not greatly written
 -^could be desirable and could not be desirable

 
 */


int main()
{
    //make below data into structure
    int rd_time = 0; //read time from file
    int *read_time = &rd_time;

    int command = 0; //FETCH, READ, or WRITE. Int for now.
    int *command_type = &command;

    long long addrs = 0x0000000000000000; //Memory address being written, read, or fetched to/from
    long long *address = &addrs;

    char userInput[2];
    int print = 0; //print option. Change this to bool

    printf("Welcome to the preliminary parser!\n");
    printf("Press P to print parsed values\nPress another key to merely read file");
    printf("\nEnter here: ");

    //fgets(userInput,2,stdin);   //User inputs option.
    scanf("%s", &userInput[0]);

    //does the user want to print, or just obtain data
    if ((!strncmp(userInput, "p", 1)) || (!strncmp(userInput, "P", 1)))
    {
        print = 1;
    }
    else
    {
        print = 0;
    }
    
    //read file function
    readfile(read_time, command_type, address, print);

    return 0;
}

void readfile(int *read_time, int *command_type, long long *address, int print)
{
	struct row_info *tempRow;
	struct row_info *currentRow;
	struct row_info *firstRow;
	char file_name[SIZEARRAYS] = ""; //will read in the name of the file.
	FILE *fp;                        //file pointer
	char str[100];                   //string for each row in file
	char hex_string[100];              //hexidecimal string to be converted to long
	char request[SIZEARRAYS];        //string to print type of request

	//time digits one through three to be converted to integer
	//current text file uses just two digits for time so far
	int count, T_count, A_count;
	int line = 1;

	//user input filename
	printf("Enter File Name: ");
	scanf("%s", file_name);

	//open file
	fp = fopen(file_name, "r");

	while (fp == NULL)
	{ //file open operation check

		printf("\nFile open not successful.\n\n");

		// Allow the user to attempt another filename
		// file_name[1] = '_';
		printf("(Press \"Q\" to quit)\nAttempt another filename: ");
		scanf("%s", file_name);

		// if the user inputs 'q' or 'Q' followed by carriage return, return
		if (tolower(file_name[0]) == 'q' && file_name[1] == '\0')
		{
			fclose(fp);
			fp = NULL;
			return;
		}
		else
		{
			fp = fopen(file_name, "r");
		}
	}

	printf("\n\n");
	if (print == 1)
	{
		printf("Requests are as follows:\n\n");
	}

	//while not the end of file
	while ((fgets(str, 128, fp) != NULL))
	{
		count = 0;
		T_count = 0;
		A_count = 0;

		// make a new node in the doubly linked list
		if ((tempRow = malloc(sizeof(struct row_info))) != NULL)
		{
			// first row, points to null on both ends, is first and last row
			if (line == 1)
			{
				// establish tail row
				firstRow = tempRow;
				tempRow->prev = NULL;
				tempRow->next = NULL;
			}
			// all other rows, prev row points to new row, new row points to prev
			else
			{
				tempRow->prev = currentRow;
				currentRow->next = tempRow;
			}
			// new head row and prev row becomes new row
			currentRow = tempRow;
			currentRow->index = line;
			currentRow->next = NULL;
		}
		else
		{
			printf("\nCould not allocate data for rows...\n\n");
			fclose(fp);
			return;
		}

		// read beginning portion of row to eliminate white space
		while (str[count] == ' ' || str[count] == '\t')
		{
			count++;
		}

		// opposite procedure to find time
		while (str[count] != ' ' && str[count] != '\t')
		{
			currentRow->time[T_count] = str[count];
			T_count++;
			count++;
		}

		// burn through whitespace again
		while (str[count] == ' ' || str[count] == '\t')
		{
			count++;
		}

		// find command value
		while (str[count] != ' ' && str[count] != '\t')
		{
			currentRow->command = str[count] - '0';
			//printf("\n%d\n", currentRow->command);
			count++;
		}

		// eliminate white space
		while (str[count] == ' ' || str[count] == '\t')
		{
			count++;
		}

		// opposite procedure to find address
		while (str[count] != ' ' && str[count] != '\t')
		{
			if (str[count] == '0' && tolower(str[count + 1]) == 'x')
			{
				count += 2;
			}

			currentRow->address[A_count] = str[count];
			A_count++;
			count++;
		}

		//PUT ALL THIS BELOW IN ANOTHER FUNCTION
		//ONE FUNCTION FOR TWO DIGIT TIME AND THREE DIGIT
		//WILL INCREASE READABILITY AND SENSE
		// 2 digits file as is now

		*read_time = atoi(currentRow->time);


		//printf("\n%d\n", currentRow->command);
		*command_type = currentRow->command;

		strcpy(hex_string, &currentRow->address[0]);
		//printf(hex_string);
		sscanf(&hex_string[0], "%llx", address);

		//CHANGE THIS, THIS IS DUMB
		if (currentRow->command == 0)
		{
			strcpy(request, "READ");
		}
		else if (currentRow->command == 1)
		{
			strcpy(request, "WRITE");
		}
		else if (currentRow->command == 2)
		{
			strcpy(request, "FETCH");
		}

		if (print == 1)
		{
			printf("Time = %d, Command attempt = %s, Memory Address = 0x%llx\n", *read_time, request, *address);
		}
		line++;
	}
	printf("\n");
	fclose(fp);

	//while ()

	fp = NULL;
}

// Function to concatenate
// three integers into one
int concat(int a, int b)
{

    char s1[SIZEARRAYS];
    char s2[SIZEARRAYS];

    // Convert all three of the integers to string
    sprintf(s1, "%d", a);
    sprintf(s2, "%d", b);

    // Concatenate both strings
    strcat(s1, s2);

    // Convert the concatenated string
    // to integer
    int d = atoi(s1);

    // return the formed integer
    return d;
}
