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
#include <limits.h>
#include "wrappers.h"

// Parameters
#define CMD_QUEUE_SIZE	16

/*
 * Helper function declarations.
 */ 
char* parseArgs(int argc, char** argv);
void Printf(char* format, ...);
void Fprintf(FILE* stream, char* format, ...);
inputCommandPtr_t peakCommand(int index);
void printRemoval(inputCommandPtr_t item);
void garbageCollection(void);
void* queueRemove(queuePtr_t queue, unsigned index);
unsigned long long getTimeJump();
int checkNewCommand();

/*
 * Global variables
 */
unsigned long long currentTime;
queuePtr_t commandQueue;
parser_t *parser;


int main(int argc, char** argv)
{
	//Parse arguments we're passing from on the command line
	char* inputFile = parseArgs(argc,argv);
	if (inputFile == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: Could not find unique file name in command line arguments\n");
		return -1;
	}

	//Init parser
	parser = initParser(inputFile);
	if (parser == NULL)
	{
		Fprintf(stderr, "Error in mem_sim:Failed to initialize parser.\n");
		return -1;
	}

	//Initialize pointer to command line that is used as a go between the parser and the queue
	
	//Init queue
	commandQueue = create_queue(CMD_QUEUE_SIZE);
	if (commandQueue == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: Could not create command queue.\n");
		return -1;
	}

	//Init DIMM

	//Initialize global time variable
	currentTime = 0;
	
	#ifdef DEBUG
	Printf("mem_sim: Completed initializations. Starting simulation.\n");
	#endif

	//Main operating loop
	while(1)
	{
		#ifdef DEBUG
			Printf("*************************************************************************\n");
			Printf("mem_sim: Top of operating loop. Status is as follows:\n");
			Printf("Current Time: %llu\n", currentTime);
			Printf("State of parser: %s\n", getParserState(parser->lineState));
			Printf("Size of command Queue: %d\n", commandQueue->size);
			if (!is_empty(commandQueue))
			{
				Printf("Printing command queue:\n");
				print_queue(commandQueue, 1, true);
			}
		#endif
		
		//Check for new command
		if (checkNewCommand() != 0)
		{
			Fprintf(stderr, "Error in mem_sim: Failed checking for next command.\n");
			garbageCollection();
			return -1;
		}

		//TODO this will turn into a scan to find commands that have waited tCL and tBURST since their read/write
		//     was issued
		#ifdef DEBUG
			Printf("mem_sim: Scanning age of commands in queue\n");
		#endif
		// FOR EACH command in the queue:
		for (int i = 1; i <= commandQueue->size; i++)
		{	
			// IF its age is >= 100, remove it and print output message
			if (getAge(i, commandQueue) >= 100)
			{
				#ifdef DEBUG
					Printf("mem_sim: Found old entry at index %d with age %llu\n",i,getAge(i,commandQueue));
				#endif
				printRemoval(peakCommand(i));
				void* oldEntry = queueRemove(commandQueue, i);
				free(oldEntry);
				i--;
			}
		}

		unsigned long long timeJump = getTimeJump();
	
		// If the command queue is empty and the parser is at EOF, then the simulation is done
		if (is_empty(commandQueue) && parser->lineState == ENDOFFILE)
		{
			Printf("At time %llu, last command removed from queue. Ending simulation.\n", currentTime);
			break;
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
			free(remove_queue_item(1, commandQueue));
		}
	}
	
	if(parser)
	{
		cleanParser(parser);
	}
}

/**
 * @fn		checkNewCommand
 * @brief	Loads the next command from the parser into the command queue
 *
 * @detail	If the command queue isn't full, passes the currentTime to the parser and requests a new command. If a new
 *		command is returned, it is added to the command queue.
 * @returns	0 if no errors occurred. -1 otherwise.
 */
int checkNewCommand()
{
	if (is_full(commandQueue) || parser->lineState == ENDOFFILE)
	{
		return 0;
	}

	// If the command queue isn't full and the parser hasn't reached EOF
	//Ask parser for next command at the current time
	inputCommandPtr_t currentCommandLine = getCommand(parser, currentTime);
	#ifdef DEBUG
		Printf("mem_sim.checkNewCommand(): Checked parser for new command.\n");
	#endif

	//If the pointer isn't NULL, add the command to the command queue
	if (currentCommandLine != NULL)
	{
		#ifdef DEBUG
			Printf("mem_sim.checkNewCommand():Adding command at trace time %llu to command queue.\n", currentCommandLine->cpuCycle);
		#endif
		// Current command line is ready for the current time to be added to the queue
		if(insert_queue_item(commandQueue, (void*)currentCommandLine) == NULL)
		{
			Fprintf(stderr,"Error in mem_sim.checkNewCommand(): Failed to insert command into command queue.\n");
			return -1;
		}
	}
	return 0;
}

/**
 * @fn		getTimeJump
 * @brief	Calculate time until another event can happen.
 *
 * @detail	Calculates time until next command can be issued to DIMM. If the command queue isn't empty,
 *		also calculates time until next command from parser. Returns the smallest of the two times.
 * @returns	Time until next decision simulation has to make
 */
unsigned long long getTimeJump()
{
	unsigned long long timeJump = ULLONG_MAX;
	// Check age of oldest entry in queue
	if (!is_empty(commandQueue))
	{
		#ifdef DEBUG
			Printf("mem_sim: Checking age of oldest command in queue\n");
		#endif
		timeJump = 100 - getAge(1, commandQueue);
		#ifdef DEBUG
			Printf("mem_sim: Age of oldest command is %llu, setting time jump to %llu\n", getAge(1, commandQueue), timeJump);
		#endif
	}
	#ifdef DEBUG
		Printf("mem_sim: Parser line state = %s\n", getParserState(parser->lineState));
	#endif
	// Ask parser when next command arrives (this would be nice but not required by saturday)
	if (parser->lineState != ENDOFFILE && !is_full(commandQueue))
	{
		#ifdef DEBUG
			Printf("mem_sim: Time of next line is %llu\n", parser->nextLineTime);
		#endif
		if (parser->nextLineTime <= currentTime) //If we have a backlog of commands
		{
			timeJump = 1; //Time must advance by at least one
		}
		else
		{
			// Determine which of the previous two times is smaller
			timeJump = (timeJump < (parser->nextLineTime-currentTime)) ? timeJump : (parser->nextLineTime-currentTime);
		}
	}
	return timeJump;
}

/**
 * @fn		queueRemove
 * @brief	Wrapper function for removing items from a 'queue'
 *
 * @detail	Removes an item from the queue at the given index and verifies
 *		that a non-NULL pointer was returned.
 * @param	queue	Target queue
 * @param	index	Location  of item in queue to be removed (starts at 1)
 * @returns	Pointer to the item removed from the queue.
 */
void* queueRemove(queuePtr_t queue, unsigned index)
{
	#ifdef DEBUG
		Printf("mem_sim:Removing item from queue at index %u\n",index);
	#endif
	void* oldItem = remove_queue_item(index, queue);
	if (oldItem == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: Failed to remove item from at queue at index %u\n", index);
	}
	return oldItem;
}

void printRemoval(inputCommandPtr_t item)
{
	Printf("\nAt time %llu, removed the following command:\n", currentTime);
	Printf("Trace Time:%llu, Command:%s, Addr:0x%llX\n",item->cpuCycle, getCommandString(item->command), item->address);
}

/**
 * @fn		peakCommand
 * @brief	Wrapper function for peaking at items in the command queue
 *
 * @param	index	Index that will be peaked in commandQueue
 * @returns	Pointer to the queue item at the index.
 */
inputCommandPtr_t peakCommand(int index)
{
	inputCommandPtr_t item = (inputCommandPtr_t)peak_queue_item(index, commandQueue);
	if (item == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: Invalid reference to command queue at index %u.\nDumping contents of command queue to stdout.\n", index);
		print_queue(commandQueue, index, true);
		garbageCollection();
		exit(EXIT_FAILURE);
	}
	return item;
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
