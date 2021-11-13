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
#include <limits.h>

/*
 * Helper function declarations.
 */ 
char* parseArgs(int argc, char** argv);
void Printf(char* format, ...);
void Fprintf(FILE* stream, char* format, ...);
queueItemPtr_t peakCommand(int index);
void printRemoval(queueItemPtr_t item);
void garbageCollection(void);

/*
 * Global variables
 */
unsigned long long currentTime;
queuePtr_t commandQueue;


int main(int argc, char** argv)
{
	//Parse arguments we're passing from on the command line
	char* inputFile = parseArgs(argc,argv);
	if (inputFile == NULL)
	{
		Printf("Error in mem_sim: Could not find unique file name in command line arguments\n");
		return -1;
	}

	//Init parser
	parserPtr_t parser = initParser(inputFile);
	parser_state_t parser_state;
	
	//Initialize pointer to command line that is used as a go between the parser and the queue
	inputCommandPtr_t currentCommandLine = NULL;
	
	//Init queue
	commandQueue = create_queue();
	if (commandQueue == NULL)
	{
		Printf("Error in mem_sim: Could not create command queue.\n");
		return -1;
	}

	//Init DIMM

	//Initialize global time variable
	currentTime = 0;
	
	#ifdef DEBUG
	Printf("mem_sim: Completed initializations. Starting simulation.\n");
	#endif

	//Main operating loop
	//***********************************************************************************************
	//
	// TODO FOR SATURDAY:
	// WHILE parser isn't finished OR command queue isn't empty:
	while(1)
	{
		#ifdef DEBUG
		Printf("mem_sim: Top of operating loop. Status is as follows:\n");
		Printf("Current Time: %llu\n", currentTime);
		Printf("State of parser: %d\n", getCommand(parser->lineState));
		Printf("Size of command Queue: %d\n", commandQueue->size);
		if (!is_empty(commandQueue))
		{
			Printf("Printing command queue:\n");
			print_queue(commandQueue, 1, true);
		}
		#endif
	// 	IF queue isn't full, pass parser current time and pointer to a inputCommand_t
	//     and IF the we are not at the end of the file
		if ((is_full(commandQueue) == false) && (parser_state != ENDOFFILE))
		{
	// 		IF that pointer returns non-NULL, add it to queue
			parser_state = getLine(parser, currentCommandLine, currentTime);
			#ifdef DEBUG
				Printf("mem_sim:Checked parser for new command. Returned state was %d\n",getCommand(parser_state));
			#endif
			if (parser_state == PARSE_ERROR)
			{
				Fprintf(stderr, "Error in mem_sim: Parser unable to parse line.\n");
				garbageCollection();
				return -1;
			}
			else if (parser_state == VALID)
			{
				#ifdef DEBUG
					Printf("mem_sim:Adding command at trace time %llu to command queue.\n", currentCommandLine->cpuCycle);
				#endif
				// Current command line is ready for the current time to be added to the queue
				if(insert_queue_item(commandQueue, currentCommandLine) == NULL)
				{
					Fprintf(stderr,"Error in mem_sim: Failed to insert command into command queue.\n");
					garbageCollection();
					return -1;
				}
			}
		}
		// FOR EACH command in the queue:
		for (int i = 1; i <= commandQueue->size; i++)
		{
			#ifdef DEBUG
				Printf("mem_sim: Scanning age of commands in queue\n");
			#endif
			queueItemPtr_t current = peakCommand(i);
			// IF its age is >= 100, remove it and print output message
			if (current->age >= 100)
			{
				#ifdef DEBUG
					Printf("mem_sim: Found old entry at index %d with age %llu\n",i,current->age);
				#endif
				printRemoval(current);
				remove_queue_item(i, commandQueue);
				i--;
			}
		}
		if (is_empty(commandQueue) && parser_state == ENDOFFILE)
		{
			Printf("At time %llu, last command removed from queue. Ending simulation.\n");
			break;
		}

		unsigned long long timeJump = ULLONG_MAX;
		// Check age of oldest entry in queue
		if (!is_empty(commandQueue))
		{
			#ifdef DEBUG
				Printf("mem_sim: Checking age of oldest command in queue\n");
			#endif
			queueItemPtr_t top = peakCommand(1);
			timeJump = 100 - top->age;
			#ifdef DEBUG
				Printf("mem_sim: Age of oldest command is %llu, setting time jump to %llu\n", top->age, timeJump);
			#endif
		}
		#ifdef DEBUG
			Printf("mem_sim: Parser line state = %d\n", parser->lineState);
		#endif
		// Ask parser when next command arrives (this would be nice but not required by saturday)
		if (parser->lineState != ENDOFFILE)
		{
			#ifdef DEBUG
				Printf("mem_sim: Time of next line is %llu\n", parser->nextLineTime);
			#endif
			// Determine which of the previous two times is smaller
			timeJump = (timeJump < parser->nextLineTime) ? timeJump : parser->nextLineTime;
		}
		#ifdef DEBUG
			Printf("mem_sim: Time jump will be %llu\n", timeJump);
		#endif
		// Check if the time jump would take us past the limit of the simulation
		if (currentTime + timeJump < currentTime)
		{
			Printf("Simulation exceeded max simulation time of %llu. Ending simulation", ULLONG_MAX);
			break;
		}
		// Advance current time by that calculated time
		currentTime += timeJump;
		#ifdef DEBUG
			Printf("mem_sim: Aging command queue\n");
		#endif
		// Age items in queue by time advanced
		age_queue(commandQueue, timeJump);
		// Loop back to start of WHILE loop
	}
	// garbage collection for queue and parser

	garbageCollection();
	return 0;
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
 * @fn		garbageCollection
 * @breif	Free up dynamically allocated memory and close file descriptors
 */
void garbageCollection()
{
	if(commandQueue)
	{
		while (!is_empty(commandQueue))
		{
			remove_queue_item(1, commandQueue);
		}
	}
}

void printRemoval(queueItemPtr_t item)
{
	Printf("\nAt time %llu, removed the following command inserted at %llu\n", currentTime, currentTime-item->age);
	Printf("Trace Time:%llu, Command:%d, Addr:%llu\n",item->command->cpuCycle, item->command->command, item->command->address);
}

/**
 * @fn		peakCommand
 * @brief	Wrapper function for peaking at items in the command queue
 *
 * @param	index	Index that will be peaked in commandQueue
 * @returns	Pointer to the queue item at the index.
 */
queueItemPtr_t peakCommand(int index)
{
	queueItemPtr_t item = peak_queue_item(index, commandQueue);
	if (item == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: Invalid reference to command queue at index %d.\nDumping contents of command queue to stdout.\n", index);
		print_queue(commandQueue, index, true);
		garbageCollection();
		exit(EXIT_FAILURE);
	}
	return item;
}

/**
 * @fn		Fprintf
 * @brief	Wrapper function for fprintf to catch errors
 *
 * @param	stream	fd of the target output file
 * @param	format	A format string provided to printf
 * @param	...	All other variables required for the given format string
 */
void Fprintf(FILE* stream, char* format, ...)
{
	va_list args;
	va_start (args, format);
	int result = vfprintf(stream, format, args);
	va_end(args);

	if (result < 0) //If the print was not successful, alert user via stderr and exit
	{
		perror("Error in mem_sim: Error calling fprintf()");
		garbageCollection();
		exit(EXIT_FAILURE);
	}
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
		garbageCollection();
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
