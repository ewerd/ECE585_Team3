/**
 * @file	mem_sim.c
 * @brief	Top level memory simulator for ECE 585 final project.
 *
 * @detail	TODO
 *
 * @date	TODO
 * @authors	TODO
 */

#include <stdarg.h>
#include "./queueADT/mem_queue.h"
#include "./parser/parser.h"
#include "./dimm/dimm.h"
#include <limits.h>
#include "wrappers.h"

// Parameters
#define CMD_QUEUE_SIZE 16
#define BANK_GROUPS 4
#define BANKS_PER_GROUP 4
#define ROWS_PER_BANK 32768

/*
 * Helper function declarations.
 */ 
void initSim(int argc, char **argv);
char *parseArgs(int argc, char **argv);
inputCommandPtr_t peakCommand(int index);
void garbageCollection(void);
void *queueRemove(queuePtr_t queue, unsigned index);
unsigned long long advanceTime(void);
unsigned long long getTimeJump(void);
int updateCommands(void);
void serviceCommands(void);
bool inOrderExecution(bool* cmdsRdy);
int sendMemCmd(inputCommandPtr_t command);
unsigned scanCommands(bool* cmdsRdy);
int updateCmdState(inputCommandPtr_t command);

#ifdef VERBOSE
void writeOutput(unsigned long long delay, const char *format, ...);
void printOutput(void);
#endif

/*
 * Global variables
 */
unsigned long long currentTime;
queuePtr_t commandQueue;
queuePtr_t outputBuffer;
parser_t *parser;
dimm_t *dimm;
FILE *output_file;

int main(int argc, char **argv)
{
	initSim(argc, argv); // Init structures

	// Initialize global time variable
	currentTime = 0;

	// output_file = fopen("output.txt", "w");

	#ifdef DEBUG
	Printf("mem_sim: Completed initializations. Starting simulation.\n");
	#endif

	// Main operating loop
	while (1)
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
		// Update command queue
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
		
		#ifdef DEBUG
		if (!is_empty(commandQueue))
		{
			Printf("mem_sim: commandQueue after updating commands:\n");	
			print_queue(commandQueue, 1, true);
		}
		#endif

		#ifdef VERBOSE
		printOutput();
		#endif

		// Advance time. If end conditions met, 0 will be returned.
		if (advanceTime() == 0)
		{
			break; // Simulation complete, exit while loop
		}
		#ifdef VERBOSE
		printOutput();
		#endif
	} // End MOL

	#ifdef VERBOSE
	if (!is_empty(outputBuffer))
	{
		age_queue(outputBuffer,getAge(outputBuffer->size, outputBuffer)); 
		printOutput();
	}
	#endif
	// garbage collection for queue and parser
	garbageCollection();

	return 0; // End
}

/**
 * @fn		garbageCollection
 * @breif	Free up dynamically allocated memory and close file descriptors
 */
void garbageCollection()
{
	fclose(output_file);
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
void initSim(int argc, char **argv)
{
	// Parse arguments we're passing from on the command line
	char *inputFile = parseArgs(argc, argv);
	if (inputFile == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim(): Could not find unique file name in command line arguments\n");
		exit(EXIT_FAILURE);
	}

	// Init parser
	parser = initParser(inputFile);
	if (parser == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim():Failed to initialize parser.\n");
		exit(EXIT_FAILURE);
	}

	// Init queue
	commandQueue = create_queue(CMD_QUEUE_SIZE);
	if (commandQueue == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.iniSim(): Could not create command queue.\n");
		goto DEINIT_PARSER;
	}

	// Init output buffer
	outputBuffer = create_queue(UINT_MAX);
	if (outputBuffer == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.initSim(): Could not create output buffer.\n");
		goto DEINIT_CMDQUEUE;
	}

	// Init DIMM
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
	// Ask parser for next command at the current time
	inputCommandPtr_t currentCommandLine = getCommand(parser, currentTime);
	#ifdef DEBUG
	Printf("mem_sim.updateCommands(): Checked parser for new command.\n");
	#endif

	// If the pointer isn't NULL, add the command to the command queue
	if (currentCommandLine != NULL)
	{

		#ifdef DEBUG
		Printf("mem_sim.updateCommands():Adding command at trace time %llu to command queue.\n", currentCommandLine->cpuCycle);
		#endif
		// Current command line is ready for the current time to be added to the queue
		if (insert_queue_item(commandQueue, (void *)currentCommandLine) == NULL)
		{
			Fprintf(stderr, "Error in mem_sim.updateCommands(): Failed to insert command into command queue.\n");
			return -1;
		}
		#ifdef VERBOSE
		writeOutput(0, "Added new request to queue:\n Time:%llu Type:%6s Address:%#010llX, Group:%u, Bank:%u, Row:%5u, Upper Column:%3u", currentCommandLine->cpuCycle, getCommandString(currentCommandLine->operation), currentCommandLine->address, currentCommandLine->bankGroups, currentCommandLine->banks, currentCommandLine->rows, currentCommandLine->upperColumns);
		#endif
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
		#ifdef DEBUG
		Printf("At time %llu, last command removed from queue. Ending simulation.\n", currentTime);
		#endif
		return 0;
	}

	#ifdef DEBUG
	Printf("mem_sim.advanceTime(): Time jump will be %llu\n", timeJump);
	#endif
	// Check if the time jump would take us past the limit of the simulation
	if (currentTime + timeJump < currentTime)
	{
		Fprintf(stderr, "Simulation exceeded max simulation time of %llu. Ending simulation", ULLONG_MAX);
		currentTime = ULLONG_MAX;
		return 0;
	}
	// Advance current time by that calculated time
	currentTime += timeJump;
	#ifdef DEBUG
	Printf("mem_sim.advanceTime(): Aging command queue\n");
	#endif
	// Age items in queue by time advanced
	age_queue(commandQueue, timeJump);
	#ifdef VERBOSE
	age_queue(outputBuffer, timeJump);
	#endif
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
	#ifdef VERBOSE
	int nextOutput = getAge(1, outputBuffer);
	timeJump = (timeJump < nextOutput) ? timeJump : nextOutput;
	#endif
	#ifdef DEBUG
	Printf("mem_sim.getTimeJump(): Parser line state = %s\n", getParserState(parser->lineState));
	#endif
	// Ask parser when next command arrives (this would be nice but not required by saturday)
	if (parser->lineState != ENDOFFILE && !is_full(commandQueue))
	{
		#ifdef DEBUG
		Printf("mem_sim.getTimeJump(): Time of next line is %llu\n", parser->nextLineTime);
		#endif
		if (parser->nextLineTime <= currentTime) // If we have a backlog of commands
		{
			timeJump = 1; // Time must advance by at least one
		}
		else
		{
			// Determine which of the previous two times is smaller
			timeJump = (timeJump < (parser->nextLineTime - currentTime)) ? timeJump : (parser->nextLineTime - currentTime);
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
void *queueRemove(queuePtr_t queue, unsigned index)
{
	#ifdef DEBUG
	Printf("mem_sim:Removing item from queue at index %u\n", index);
	#endif
	void *oldItem = remove_queue_item(index, queue);
	if (oldItem == NULL)
	{
		Fprintf(stderr, "Error in mem_sim.queueRemove(): Failed to remove item from queue at index %u\n", index);
		print_queue(queue, index, true);
		garbageCollection();
		exit(EXIT_FAILURE);
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
char *parseArgs(int argc, char **argv)
{
	char *fileName = NULL;
	char *outFile = NULL;
	bool out_flag = false; 

	for (int i = 1; i < argc; i++) // For each string in argv
	{
		// Once we add flags they can go here like this example
		/*
		if (!strcmp(argv[i], "-step")) //If the -step flag is asserted
		{
			if (isNumber(argv[i+1])) //Verify the user specified a number after flag
			{
				i++; //Step over next argument since we're using it for this
				timestep = strtoull(argv[i], 0, 0);
			}
			else
				expectedNumber("step", argv[i+1]); //If no number follows, alert user. Default to 1.
		}
		*/
		if ((argv[i][0] != '-') && i == 1)
		{
			if (fileName == NULL)
			{
				fileName = argv[i];
			}
			else
			{
				Printf("Error in mem_sim: Multiple input file names provided:\n%s\n%s\n", fileName, argv[i]);
				return NULL;
			}
		}
		if (!strcmp(argv[i], "-o")) // If the -out flag is asserted
		{
			if (i + 1 < argc) //makes sure we won't go out of bounds in strstr
			{
				if (strstr(argv[i + 1], ".txt") != NULL)  // Verify the user specified a filename
				{
					
					outFile = argv[i + 1];
					output_file = fopen(outFile, "w");
					#ifdef DEBUG
					Printf("mem_sim.parseArgs():Output file flag detected. File Format Correct. Print to %s\n", outFile);
					#endif
				}
				else
				{	
					
					output_file = fopen("output.txt", "w");
					#ifdef DEBUG
					Printf("mem_sim.parseArgs()Output file flag detected. File Format Not Correct. Defualt to 'output.txt'\n");
					#endif
				}
			}
			else
			{
				
				output_file = fopen("output.txt", "w");
				#ifdef DEBUG
				Printf("mem_sim.parseArgs():Output file flag detected. File name not provided. Defualt to 'output.txt'\n");
				#endif
			}
			out_flag = true; 
		}
		
		if(!out_flag && i == argc -1)
			{
				
				output_file = stdout;
				#ifdef DEBUG
				Printf("mem_sim.parseArgs():Output file flag NOT detected. Printing to stdout\n\n");
				#endif
			}
	}
	// Check bounds on parameters and assert defaults and/or print error messages
	if (fileName == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: No input file name provided.\n");
	}

	return fileName;
}

/**
 * @fn		serviceCommands
 * @brief	Determines and issues commands to the DIMM for the current clock cycle
 *
 * @detail	Updates the state of memory requests in the commandQueue and then calls
 *		the scheduling function to determine which command, if any, should be
 *		issued to the DIMM.
 */
void serviceCommands()
{
	bool cmdsRdy[commandQueue->size];
	unsigned numRdyCmds = scanCommands(cmdsRdy);
	#ifdef DEBUG
		Printf("mem_sim.serviceCommands(): %u command(s) ready\n", numRdyCmds);
		Printf("mem_sim.serviceCommands(): cmdsRdy array:\n[");
		for (int i = 1; i <= commandQueue->size; i++)
		{
			if (cmdsRdy[i-1])
				Printf("t");
			else
				Printf("f");
			if (i != commandQueue->size)
				Printf("\t");
		}
		Printf("]\n");
	#endif
	if (numRdyCmds > 0)
	{
		inOrderExecution(cmdsRdy);
		//More scheduling functions can be called here if out of order execution is desired
	}
}

/**
 * @fn		inOrderExecution
 * @brief	Sends command to DIMM with a schedule for in order execution
 *
 * @detail	Determines which command, if any, should be issued to the DIMM this clock tick
 * @param	cmdsRdy	An array of bools. A true value means that the corresponding command at the same index
 *			in the commandQueue is ready to be serviced by the DIMM.
 * @return	True if a command is issued to the DIMM. False otherwise.
 */
bool inOrderExecution(bool* cmdsRdy)
{
	bool bankGroupTouched[4] = {false, false, false, false};
	// FOR EACH command in the queue:
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		inputCommandPtr_t command = (inputCommandPtr_t)peak_queue_item(i, commandQueue);
		if (command->nextCmd != REMOVE)//Ignore commands that are just waiting for data. We're done with them
		{
			if (bankGroupTouched[command->bankGroups] == false && cmdsRdy[i-1] == true)
			{
				#ifdef DEBUG
				Printf("mem_sim: Servicing command at index %d\n",i);
				#endif
				#ifdef VERBOSE
				writeOutput(0, "Priority request identified at index %u: Time:%llu Type:%6s Group:%u, Bank:%u, Row:%u, Upper Column:%u", i, command->cpuCycle, getCommandString(command->operation), command->bankGroups, command->banks, command->rows, command->upperColumns);
				#endif
				setAge(i, sendMemCmd(command), commandQueue);
				return true;
			}
			bankGroupTouched[command->bankGroups] = true;
		}
	}
	return false;
}

/**
 * @fn		sendMemCmd
 * @brief	Sends a command to the DRAM
 *
 * @detail	Depending on the commands nextCmd member, issues a command to the DRAM
 * @param	command	The memory request being serviced.
 * @return	A positive integer representing the CPU clock cycles until the command is completed.
 * @warning	Warning: Passing a command to this function that the DIMM is not ready to handle will
 *		likely result in prematurely ending the simulation on an error. See scanCommands()
 *		and updateCmdState()
 */
int sendMemCmd(inputCommandPtr_t command)
{
	int retVal;
	switch(command->nextCmd)
	{
		case ACCESS:
			command->nextCmd = REMOVE;
			if (command->operation == WRITE)
			{
				retVal = dimm_write(dimm, command->bankGroups, command->banks, command->rows, currentTime);
				#ifdef VERBOSE
				writeOutput(0, "Group %u, Bank %u received write command to row %u, upper column %u", command->bankGroups, command->banks, command->rows, command->upperColumns);
				writeOutput(retVal-8, "Group %u, Bank %u has begun latching data and storing in row %u, upper column %u", command->bankGroups, command->banks, command->rows, command->upperColumns);
				writeOutput(retVal, "Group %u, Bank %u has completed writing to row %u, upper column %u", command->bankGroups, command->banks, command->rows, command->upperColumns);
				#else
				Fprintf(output_file, "%'26llu\tWR  %u %u %u\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
				#endif
			}
			else
			{
				retVal = dimm_read(dimm, command->bankGroups, command->banks, command->rows, currentTime);
				#ifdef VERBOSE
				writeOutput(0, "Group %u, Bank %u received read command to row %u, upper column %u", command->bankGroups, command->banks, command->rows, command->upperColumns);
				writeOutput(retVal-8, "Group %u, Bank %u has begun bursting data from row %u, upper column %u", command->bankGroups, command->banks, command->rows, command->upperColumns);
				writeOutput(retVal, "Group %u, Bank %u has completed burst from row %u, upper column %u", command->bankGroups, command->banks, command->rows, command->upperColumns);
				#else
				Fprintf(output_file, "%'26llu\tRD  %u %u %u\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
				#endif
			}
			break;
		case ACTIVATE:
			command->nextCmd = ACCESS;
			retVal = dimm_activate(dimm, command->bankGroups, command->banks, command->rows, currentTime);
			#ifdef VERBOSE
			writeOutput(0, "Group %u, Bank %u has begun activating row %u", command->bankGroups, command->banks, command->rows);
			writeOutput(retVal, "Group %u, Bank %u has completed activating row %u", command->bankGroups, command->banks, command->rows);
			#else
			Fprintf(output_file, "%'26llu\tACT %u %u %u\n", currentTime, command->bankGroups, command->banks, command->rows);
			#endif
			break;
		case PRECHARGE:
			command->nextCmd = ACTIVATE;
			retVal = dimm_precharge(dimm, command->bankGroups, command->banks, currentTime);
			#ifdef VERBOSE
			writeOutput(0, "Group %u, Bank %u has closed row %u and begun precharging", command->bankGroups, command->banks, dimm->group[command->bankGroups]->bank[command->banks]->row);
			writeOutput(retVal, "Group %u, Bank %u has completed precharging and is ready to activate a row", command->bankGroups, command->banks);
			#else
			Fprintf(output_file, "%'26llu\tPRE %u %u\n", currentTime, command->bankGroups, command->banks);
			#endif
			break;
		default:
			retVal = -1;
	}
	if (retVal > 0)
		return retVal;

	Fprintf(stderr, "Error in mem_sim.sendMemCmd(): Invalid command serviced. retVal = %d\n", retVal);
	garbageCollection();
	exit(EXIT_FAILURE);
}

/**
 * @fn		scanCommands
 * @brief	Updates the state of all requests in the command queue
 *
 * @detail	Updates the age (time until next action required) and next DRAM command (ACCESS, ACTIVATE, PRECHARGE, or
 *		REMOVE(from queue)) for each memory request in the command queue. Updates the array of bools to reflect
 *		which requests can be serviced this clock cycle. A value of true in cmdsRdy[x] indicates that the
 *		request in the command queue at index 'x' is ready to have a DRAM command issued.
 * @param	cmdsRdy	Array of bools updated to true if the request in the command queue at the same index is ready
 *			for a DRAM command to be issued. False otherwise
 * @return	Number of requests in the command queue that are ready to be serviced with a DRAM command
 */
unsigned scanCommands(bool* cmdsRdy)
{
	unsigned numCmdsRdy = 0;
	for (int i = 1; i <= commandQueue->size; i++)
	{
		cmdsRdy[i-1] = false;
		inputCommandPtr_t command = (inputCommandPtr_t)peak_queue_item(i, commandQueue);
		if (command->nextCmd == REMOVE) 
		{
			if (getAge(i, commandQueue) == 0)
			{
				#ifdef VERBOSE
				writeOutput(0, "Completed request from Time:%llu Type:%6s Group:%u, Bank:%u, Row:%u, Upper Column:%u", command->cpuCycle, getCommandString(command->operation), command->bankGroups, command->banks, command->rows, command->upperColumns);
				#endif
				free(queueRemove(commandQueue,i));
				i--;
			}
		}
		else
		{
			int newAge = updateCmdState(command);
			setAge(i, newAge, commandQueue);
			#ifdef DEBUG
				Printf("mem_sim: Command %u updated. nextCmd: %s, Age: %d\n", i, nextCmdToString(command->nextCmd), newAge);
			#endif
			if (newAge == 0)
			{
				#ifdef DEBUG
				Printf("mem_sim.scanCommands(): Command %d ready.\n", i);
				#endif
				cmdsRdy[i-1] = true;
				numCmdsRdy++;
			}
		}
	}
	return numCmdsRdy;
}

/**
 * @fn		updateCmdState
 * @brief	Updates an input command's next desired DRAM command and the time till it can be issued
 *
 * @param	command	Target command being updated
 * @return	0 if the command is ready to be serviced. A positive integer representing the number of CPU clock ticks until
 *		the command will be ready.
 */
int updateCmdState(inputCommandPtr_t command)
{		
	int timeTillCmd = (command->operation == WRITE) ?	
		dimm_canWrite(dimm, command->bankGroups, command->banks, command->rows, currentTime) :
		dimm_canRead(dimm, command->bankGroups, command->banks, command->rows, currentTime);
	if (timeTillCmd >= 0)
	{
		command->nextCmd = ACCESS;
		return timeTillCmd;
	}
	if (timeTillCmd == -1)
	{
		timeTillCmd = dimm_canActivate(dimm, command->bankGroups, command->banks, currentTime);
	}
	if (timeTillCmd >= 0)
	{
		command->nextCmd = ACTIVATE;
		return timeTillCmd;
	}
	if (timeTillCmd == -1)
	{
		timeTillCmd = dimm_canPrecharge(dimm, command->bankGroups, command->banks, currentTime);
	}
	if (timeTillCmd >= 0)
	{
		command->nextCmd = PRECHARGE;
		return timeTillCmd;
	}
	Fprintf(stderr, "Error in mem_sim.updateCmdState(): Ended function with error code of %d.\n", timeTillCmd);
	garbageCollection();
	exit(EXIT_FAILURE);
}

#ifdef VERBOSE
void writeOutput(unsigned long long delay, const char *format, ...)
{
	char message[1024];
	va_list argList;
	va_start(argList, format);
	int size = vsprintf(message, format, argList);
	char *entry = Malloc((size + 1) * sizeof(char));
	strcpy(entry, message);
	sorted_insert_queue(entry, delay, outputBuffer);
}

void printOutput(void)
{
	while (getAge(1, outputBuffer) == 0)
	{
		char *output = (char *)queueRemove(outputBuffer, 1);
		Fprintf(output_file, "%'llu : %s\n", currentTime, output);
		free(output);
	}
}
#endif
