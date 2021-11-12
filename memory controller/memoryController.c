/*
MemoryController.c
*/

//Main
/*
Request Input Option (P)
Request Filename
Open File
Set cycle time to 0
Check if line is empty
 - If current line is empty/there is no line
	read in line
 - Check if current time is the same as the line time
	- If line is less than current time, check if the queue is empty
	- If empty, advance time to the line time
 - Look at queue operation to see if the next operation is possible
		- If next operation is possible, take it
		- else, continue time
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "mem_queue.h"
#include "memoryController.h"

void printCurrentLine(inputCommandPtr_t currentCommandLine);

int main()
{
	// create and initiate queue
	queuePtr_t queue = create_queue();

	// Set Debug Option
	// Should set this up to be a command line argument
	bool printDebug = debugMode();

	// Open input file
	FILE *inputFile = openFile();

	// Set start time for CPU Clock Cycles
	long long cpuTime = 0;

	// Use a boolean value to determine if the file is empty or not
	bool fileEmpty = false;


	inputCommandPtr_t currentCommandLine = NULL;
	if ((currentCommandLine = malloc(sizeof(inputCommand_t))) == NULL) {
		printf("Error: Unable to malloc new command line.\n");
		exit(1);
	}



	if (printDebug == true)
	{
		printf("Requests are as follows:\n\n");
		// Read first line
		fileEmpty = readLine(inputFile, currentCommandLine);
		insert_queue_item(queue, currentCommandLine);
		print_queue(queue, 1, true);

		// attempt to set up queue
		printf("\nItems will be shown as they are parsed. After each item is parsed, it will be\n"
			"added to the queue. The entire contents of the queue will then be printed to\n"
			"to demonstrate that the queue is adding parsed elements correctly.\n\n");
		while (fileEmpty == false)
		{
			printCurrentLine(currentCommandLine);
			fileEmpty = readLine(inputFile, currentCommandLine);
			insert_queue_item(queue, currentCommandLine);
			print_queue(queue, 1, true);
		}
	}

	//while((fileEmpty == false) && (isQueueEmpty() == false))
	//{
	//	// Determine if we need to get another line
	//	
	//		// If fileEmpty == false
	//			// If current line has already been added to the queue, grab another line, check if its EOF and if it isn't reset transferred to false
	//			
	//			// Check if current cpu time is the same as the time of the current line
	//			// If same time, and queue is not full, add to queue
	//		
	//		// Else (file is empty and now we just need to focus on the rest of the queue)
	//		
	//		
	//		// end if/else
	//		
	//	// Determine what the next operation is
	//		// If queue is empty, increment to the next CPUTime for the current line
	//		
	//		// Else
	//	
	//	
	//	break;
	//	
	//	
	//	
	//}
	free(currentCommandLine);

	char indexInput[10];
	int indexRemove;

	printf("\n\nSelect an index to remove from the queue (press 'q' to quit): ");
	scanf("%s", indexInput);

	while (indexInput[0] != 'q')
	{
		indexRemove = atoi(indexInput);
		remove_queue_item(indexRemove, queue);
		print_queue(queue, 1, true);
		printf("\n\nSelect an index to remove from the queue (press 'q' to quit): ");
		scanf("%s", indexInput);
	}

    return 0;
}

bool readLine(FILE *inputFile, inputCommandPtr_t currentLine)
{
	char inputLine[100];
	int numFields;
	inputCommand_t nextLine;
	unsigned commandInt;
	
	if (fgets(inputLine, 100, inputFile) == NULL)
	{ // Returns false if it reaches the end of the file
		return true;
	}
	else 
	{
		// Not end of file
		numFields = sscanf(inputLine, " %lld %d %llx\n", &nextLine.cpuCycle, &commandInt, &nextLine.address);
		
		if (numFields != 3)
		{
			printf("Error, incorrect number of fields parsed from file.\n");
			exit(3);
		}
		nextLine.command		= (operation_t) commandInt;
		nextLine.rows 			= (nextLine.address & 0x1FFFC0000) >> 18;
		nextLine.upperColumns 	= (nextLine.address & 0x3FC00) >> 10;
		nextLine.banks 			= (nextLine.address & 0x300) >> 8;
		nextLine.bankGroups		= (nextLine.address & 0xC0) >> 6;
		nextLine.lowerColumns	= (nextLine.address & 0x38) >> 3;
		nextLine.byteSelect 	=  nextLine.address & 0x7;
		
		if (currentLine != NULL)
		{
			*currentLine = nextLine;
			return false;
		}
		else
		{
			// memory not set for currentLine
			return true;
		}
	}
}

const char* getCommand(operation_t command)
{
   switch (command) 
   {
      case   READ: return "READ";
	  case  WRITE: return "WRITE";
	  case IFETCH: return "IFETCH";
   }
   return "ERROR";
}

bool debugMode()
{
	bool debugOption = false;
	
	printf("Welcome to the updated parser!\n");
    printf("Press P to print parsed values\nPress another key to merely read file");
    printf("\nEnter here: ");
	
	// Gather input
	char userInput[10];
    if(fgets(userInput, 10, stdin) != NULL) {
		userInput[strlen(userInput) - 1] = 0; // Remove newline character
    }
	else {
		printf("Error with first input.\n");
		exit(1);
	}

    //does the user want to print, or just obtain data
    if ((!strncmp(userInput, "p", 1)) || (!strncmp(userInput, "P", 1)))
    {
		debugOption = true;
	}
    else
	{
        debugOption = false;
	}
		
	return debugOption;
}

FILE *openFile()
{
	// TO DO: Add option to input filename again if failed
	
	FILE *inputFile = NULL;
	
	//user input filename
	char file_name[20];
	printf("Enter File Name: ");
	if(fgets(file_name, 20, stdin) != NULL) {
		file_name[strlen(file_name) - 1] = 0; // Remove newline character
    }
	else {
		printf("Error with file name input.\n");
		exit(2);
	}

	//open file
	inputFile = fopen(file_name, "r");
	if (inputFile == NULL)
	{
		printf("Error opening file.\n");
		exit(3);
	}
	
	return inputFile;
}

void printCurrentLine(inputCommandPtr_t currentCommandLine)
{
	printf("Time = %10lld, Command Attempt = %6s, Address = 0x%010llX, Row = %5u, Upper Column = %3u, Bank = %2u, Bank Group = %1u, Lower Column = %1u, Byte Select = %1u\n", currentCommandLine->cpuCycle, getCommand(currentCommandLine->command), currentCommandLine->address, currentCommandLine->rows, currentCommandLine->upperColumns, currentCommandLine->banks, currentCommandLine->bankGroups, currentCommandLine->lowerColumns, currentCommandLine->byteSelect); 
}