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
#include "./dimm/dimm.h"
#include <limits.h>
#include "wrappers.h"

// Parameters
#define CMD_QUEUE_SIZE	16
#define BANK_GROUPS	4
#define BANKS_PER_GROUP	4
#define ROWS_PER_BANK	32768

/*
 * Helper function declarations.
 */ 
void initSim(int argc, char** argv);
char* parseArgs(int argc, char** argv);
void Printf(char* format, ...);
void Fprintf(FILE* stream, char* format, ...);
void OUTPUT(FILE* output_file, char* format, ...);
inputCommandPtr_t peakCommand(int index);
void garbageCollection(void);
void* queueRemove(queuePtr_t queue, unsigned index);
unsigned long long advanceTime(void);
unsigned long long getTimeJump(void);
int updateCommands(void);
void serviceCommands(void);
int sendMemCmd(inputCommandPtr_t command);

#ifdef VERBOSE
void writeOutput(char* message, unsigned long long delay);
void printOutput(void);
#endif

/*
 * Global variables
 */
unsigned long long currentTime;
queuePtr_t commandQueue;
queuePtr_t outputBuffer;
parser_t *parser;
dimm_t* dimm;
FILE *output_file; 

int main(int argc, char** argv)
{
	initSim(argc, argv); //Init structures

	//Initialize global time variable
	currentTime = 0;
	
	output_file = fopen("output.txt", "w");

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
		
		//Update command queue
		if (updateCommands() != 0)
		{
			Fprintf(stderr, "Error in mem_sim: Failed updating command queue.\n");
			garbageCollection();
			return -1;
		}
		
		if (currentTime % 2 == 0)
		{
			#ifdef DEBUG
				Printf("mem_sim: Iterating through queued commands\n");
			#endif
			serviceCommands();
		}
		
		#ifdef VERBOSE
		printOutput();
		#endif

		// Advance time. If end conditions met, 0 will be returned.
		if (advanceTime() == 0)
		{
			break; //Simulation complete, exit while loop
		}
	}//End MOL

	// garbage collection for queue and parser
	garbageCollection();
	
	return 0; //End
}

/**
 * @fn		garbageCollection
 * @breif	Free up dynamically allocated memory and close file descriptors
 */
void garbageCollection()
{
	clean_queue(commandQueue);
	clean_queue(outputBuffer);
	cleanParser(parser);
	dimm_deinit(dimm);
}

/**
 * @fn		initSim
 * @brief	Initializes all the global pointer in the simulation
 *
 */
void initSim(int argc, char** argv)
{
	//Parse arguments we're passing from on the command line
	char* inputFile = parseArgs(argc,argv);
	if (inputFile == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim(): Could not find unique file name in command line arguments\n");
		exit(EXIT_FAILURE);
	}

	//Init parser
	parser = initParser(inputFile);
	if (parser == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim():Failed to initialize parser.\n");
		exit(EXIT_FAILURE);
	}
	
	//Init queue
	commandQueue = create_queue(CMD_QUEUE_SIZE);
	if (commandQueue == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.iniSim(): Could not create command queue.\n");
		goto DEINIT_PARSER;
	}

	//Init output buffer
	outputBuffer = create_queue(UINT_MAX);
	if (outputBuffer == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim(): Could not create output buffer.\n");
		goto DEINIT_CMDQUEUE;
	}

	//Init DIMM
	dimm = dimm_init(BANK_GROUPS, BANKS_PER_GROUP, ROWS_PER_BANK);
	if (dimm == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim(): Could not initialize dimm.\n");
		goto DEINIT_OUTPUTBUFF;
	}

	return;

	DEINIT_OUTPUTBUFF:
	clean_queue(outputBuffer);
	DEINIT_CMDQUEUE:
	clean_queue(commandQueue);
	DEINIT_PARSER:
	cleanParser(parser);
	exit(EXIT_FAILURE);
}

/**
 * @fn		updateCommands
 * @brief	Loads the next command from the parser into the command queue
 *
 * @detail	If the command queue isn't full, passes the currentTime to the parser and requests a new command. If a new
 *		command is returned, it is added to the command queue.
 * @returns	0 if no errors occurred. -1 otherwise.
 */
int updateCommands()
{
	if (is_full(commandQueue) || parser->lineState == ENDOFFILE)
	{
		return 0;
	}

	// If the command queue isn't full and the parser hasn't reached EOF
	//Ask parser for next command at the current time
	inputCommandPtr_t currentCommandLine = getCommand(parser, currentTime);
	#ifdef DEBUG
		Printf("mem_sim.updateCommands(): Checked parser for new command.\n");
	#endif

	//If the pointer isn't NULL, add the command to the command queue
	if (currentCommandLine != NULL)
	{

		#ifdef DEBUG
			Printf("mem_sim.updateCommands():Adding command at trace time %llu to command queue.\n", currentCommandLine->cpuCycle);
		#endif
		// Current command line is ready for the current time to be added to the queue
		if(insert_queue_item(commandQueue, (void*)currentCommandLine) == NULL)
		{
			Fprintf(stderr,"Error in mem_sim.updateCommands(): Failed to insert command into command queue.\n");
			return -1;
		}
	}
	return 0;
}

/**
 * @fn		advanceTime
 * @brief	Advances simulation time to next event
 *
 * @detail	Checks the time of the next pending command from the trace file and the next time a command can be issued
 *		to the DIMM. Simulation time is advanced by the smaller of the two. If the simulation is complete, 0 will
 *		be returned.
 * @returns	CPU clock cycles that time has been advanced. 0 if the simulation is over.
 */
unsigned long long advanceTime()
{
	unsigned long long timeJump = getTimeJump();

	// If the command queue is empty and the parser is at EOF, then the simulation is done
	if (is_empty(commandQueue) && parser->lineState == ENDOFFILE)
	{
		Printf("At time %llu, last command removed from queue. Ending simulation.\n", currentTime);
		fclose(output_file);
		return 0;
	}

	#ifdef DEBUG
		Printf("mem_sim.advanceTime(): Time jump will be %llu\n", timeJump);
	#endif
	// Check if the time jump would take us past the limit of the simulation
	if (currentTime + timeJump < currentTime)
	{
		Printf("Simulation exceeded max simulation time of %llu. Ending simulation", ULLONG_MAX);
		return 0;
	}
	// Advance current time by that calculated time
	currentTime += timeJump;
	#ifdef DEBUG
		Printf("mem_sim.advanceTime(): Aging command queue\n");
	#endif
	// Age items in queue by time advanced
	age_queue(commandQueue, timeJump);
	return timeJump;
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
	// Check for next command that'll be ready
	if (!is_empty(commandQueue))
	{
		#ifdef DEBUG
			Printf("mem_sim.getTimeJump(): Checking age of commands in queue\n");
		#endif
		for (unsigned i = 1; i <= commandQueue->size; i++)
		{
			int cmdTimeRemaining = getAge(i, commandQueue);
			timeJump = (cmdTimeRemaining < timeJump) ? cmdTimeRemaining : timeJump;
		}

		if (timeJump < 1)
			timeJump = 1;
		#ifdef DEBUG
			Printf("mem_sim.getTimeJump(): Next command finishes in %llu\n", timeJump);
		#endif
	}
	#ifdef DEBUG
		Printf("mem_sim.getTimeJump(): Parser line state = %s\n", getParserState(parser->lineState));
	#endif
	// Ask parser when next command arrives (this would be nice but not required by saturday)
	if (parser->lineState != ENDOFFILE && !is_full(commandQueue))
	{
		#ifdef DEBUG
			Printf("mem_sim.getTimeJump(): Time of next line is %llu\n", parser->nextLineTime);
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
			
void serviceCommands()
{
	bool bankGroupTouched[4] = {false, false, false, false};
	// FOR EACH command in the queue:
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		inputCommandPtr_t command = (inputCommandPtr_t)peak_queue_item(i, commandQueue);
		if (bankGroupTouched[command->bankGroups] == false && getAge(i, commandQueue) == 0)
		{
			#ifdef DEBUG
				Printf("mem_sim: Servicing command at index %d\n",i);
			#endif
			if (command->nextCmd == REMOVE)
			{
				free(queueRemove(commandQueue,i));
				i--;
			}
			else
			{
				int newAge = sendMemCmd(command);
				if (newAge < 0)
				{
					Fprintf(stderr, "Error in mem_sim.serviceCommands(): Failed memory access.\n");
					garbageCollection();
					exit(EXIT_FAILURE);
				}
				setAge(i, newAge, commandQueue);
				return;
			}
		}
		bankGroupTouched[command->bankGroups] = true;
	}
}

int sendMemCmd(inputCommandPtr_t command)
{
	int retVal = (command->operation == WRITE) ? 
		dimm_canWrite(dimm, command->bankGroups, command->banks, command->rows, currentTime) :
		dimm_canRead(dimm, command->bankGroups, command->banks, command->rows, currentTime);
	if (retVal == 0)
	{
		command->nextCmd = REMOVE;
		if (command->operation == WRITE)
		{
			Printf("%'26llu\tWR  %u %u %u\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
			OUTPUT(output_file, "%'26llu\tWR  %u %u %u\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
			return dimm_write(dimm, command->bankGroups, command->banks, command->rows, currentTime);
		}
		else
		{
			Printf("%'26llu\tRD  %u %u %u\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
			OUTPUT(output_file,"%'26llu\tRD  %u %u %u\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
			return dimm_read(dimm, command->bankGroups, command->banks, command->rows, currentTime);
		}
	}
	if (retVal == -1)
		retVal = dimm_canActivate(dimm, command->bankGroups, command->banks, currentTime);
	if (retVal == 0)
	{
		Printf("%'26llu\tACT %u %u %u\n", currentTime, command->bankGroups, command->banks, command->rows);
		OUTPUT(output_file, "%'26llu\tACT %u %u %u\n", currentTime, command->bankGroups, command->banks, command->rows);
		return dimm_activate(dimm, command->bankGroups, command->banks, command->rows, currentTime);
	}
	if (retVal == -1)
		retVal = dimm_canPrecharge(dimm, command->bankGroups, command->banks, currentTime);
	if (retVal == 0)
	{
		Printf("%'26llu\tPRE %u %u\n", currentTime, command->bankGroups, command->banks);
		OUTPUT(output_file, "%'26llu\tPRE %u %u\n", currentTime, command->bankGroups, command->banks);
		return dimm_precharge(dimm, command->bankGroups, command->banks, currentTime);
	}
	return retVal;
}

#ifdef VERBOSE
void writeOutput(char* message, unsigned long long delay)
{
	char* entry = Malloc((strlen(message)+1)*sizeof(char));
	strcpy(entry, message);
	sorted_insert_queue(entry, delay, outputBuffer);
}

void printOutput(void)
{
	while (getAge(1, outputBuffer) <= currentTime)
	{
		char* output = (char*)queueRemove(outputBuffer, 1);
		Printf("%'4llu : %s\n", currentTime, output);
		free(output);
	}
}
#endif
