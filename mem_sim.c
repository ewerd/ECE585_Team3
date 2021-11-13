/**
 * @file	mem_sim.c
 * @brief	Top level memory simulator for ECE 585 final project.
 *
 * @detail	TODO
 *
 * @date	TODO
 * @authors	TODO
 */

#include "./queueADT/mem_queue.h"
#include "./parser/parser.h"
#include <stdarg.h>

/*
 * Helper function declarations.
 */ 
char* parseArgs(int argc, char** argv);
void Printf(char* format, ...);

int main(int argc, char** argv)
{
	//Parse arguments if we're passing them on the command line
	char* inputFile = parseArgs(argc,argv);
	if (inputFile == NULL)
	{
		Printf("Error in mem_sim: Could not find unique file name in command line arguments\n");
		return -1;
	}

	//Init parser
	//TODO Call initialization function for parser. Pass char* inputFile as argument.
	//TODO inputFile will point to a \0 terminated string that is the name of the input trace file

	//Init queue
	queuePtr_t commandQueue = create_queue();
	if (commandQueue = NULL)
	{
		Printf("Error in mem_sim: Could not create command queue.\n");
		return -1;
	}

	//Init DIMM

	//Initialize global time variable
	unsigned long long currentTime = 0;

	//Main operating loop
	//***********************************************************************************************
	//
	// TODO FOR SATURDAY:
	// WHILE parser isn't finished AND command queue isn't empty:
	// 	IF queue isn't full, pass parser current time and pointer to a inputCommand_t
	// 		IF that pointer returns non-NULL, add it to queue
	//	FOR EACH command in the queue:
	//		IF it's age is >= 100, remove it and print output message
	//	Check age of oldest entry in queue
	//	Ask parser when next command arrives (this would be nice but not required by saturday)
	//	Determine which of the previous two times is smaller
	//	Advance current time by that calculated time
	//	Age items in queue by time advanced
	//      Loop back to start of WHILE loop
	// garbage collection for queue and parser
	//
	//***********************************************************************************************

	// Everything below here are notes for final implementation

	//While queue isn't empty, ping parser for available requests at current time
	//Add each line to back of queue

	//For basic implementation, get bank group, bank, row of top request

	//Ask DIMM if that bank group and bank are ready for a command
	//Determine what next command should be and issue it to DIMM
	//If data is ready, remove request from queue and...print message? Check project description for what needs to happen

	//Determine time until DIMM is ready for next command
	//Determine time until next request arrives from parser
	//Advance time by the smaller of the two times

	//Loop back to start of MOL
}

/**
 * @fn		Printf
 * @brief	Wrapper function for printf to catch errors
 *
 * @param	format	A format string provided to printf
 * @param	...	All other variables required for the given format string
 */
void Printf(char* format, ...)
{
	va_list args;
	va_start (args, format);
	int result = vprintf(format, args);
	va_end(args);

	if (result < 0) //If the print was not successful, alert user via stderr, collect garbage, and exit
	{
		perror("Error in mem_sim: Error calling printf()");
		exit(EXIT_FAILURE);
	}
}

/**
 * @fn		parseArgs
 * @brief	Parses the arguments of the thread for flags and the input file name.
 *
 * @detail	Scans each input to see if it is setting a flag. Checks for syntax and argument bounds. Warns the user
 * 		if invalid arguments are received or out-of-bound values used but just corrects or changes values to
 * 		defaults and continues execution
 *
 * @param	argc	number of arguments to parse
 * @param	argv	array of pointers to the beginning of each
 * 			argument string
 * @return	Pointer to a string containing the input file name. NULL if no input file was provided
 */
char* parseArgs(int argc, char** argv)
{
	char* fileName = NULL;

	for (int i = 1; i < argc; i++) //For each string in argv
	{
		//Once we add flags they can go here like this example
		/*
		if (!strcmp(argv[i], "-step")) //If the -step flag is asserted
		{
			if (isNumber(argv[i+1])) //Verify the user specified a number after flag
			{
				i++;
				timestep = strtoull(argv[i], 0, 0);
			}
			else
				expectedNumber("step", argv[i+1]); //If no number follows, alert user. Default to 1.
		}
		*/
		if (argv[i][0] != '-')
		{
			if (fileName == NULL)
			{
				fileName = argv[i];
			}
			else
			{
				Printf("Error in mem_sim: Multiple input file names provided:\n%s\n%s\n",fileName,argv[i]);
				return NULL;
			}
		}
	} // End of parsing arguments

	// Check bounds on parameters and assert defaults and/or print error messages
	if (fileName == NULL)
	{
		Printf("Error in mem_sim: No input file name provided.\n");
	}

	return fileName;
}
