/**
 * @file	mem_sim.c
 * @brief	Top level memory simulator for ECE 585 final project.
 *
 * @detail	Simulates a DRAM memory controller that implements several different scheduling policies. 
 *		All policies use an open page policy and exploit bank paralellism.
 * @flags	-o <output_file>	Send output to a .txt file. If output_file is blank, will default
 *					to output.txt
 *		-stat	Displays statistics after completing the simulation. Statistics include
 *			min, max, average, and median time in queue for each type of operation
 *			as well as for the aggregate total for all commands
 *		-<policy>	Possible policies are:
 *			strict	The memory controller sticks to a strict in order scheduling
 *				fetches, reads, and writes will be serviced in the exact
 *				order that they arrive.
 *			opt	Our optimized policy. This algorithm prioritizes fetches, reads, and writes
 *				(in that order) to open rows, then fetches, reads, and writes(in that order)
 *				to other rows. To prevent starvation, each operation type has an upper
 *				threshold for time in queue and, once a request passes that threshold, it
 *				moves to the highest priority. Lower priority requests may be serviced
 *				earlier than higher priority requests but only if doing so does not slow
 *				down higher priority requests in any way. The thresholds can be set by the user
 *				with the -<fch/rd/wr> <threshold> flags or the default ones will be used.
 *			<none>	This is the default policy which gives priority to the oldest requests
 *				in the queue. Newer requests can still be serviced sooner but only if
 *				processing those requests does not slow down any older request.
 *
 * @date	Dec. 6, 2021
 * @author	Stephen Short	(steshort@pdx.edu)
 * @author	Drew Seidel	(dseidel@pdx.edu)
 * @author	Michael Weston	(miweston@pdx.edu)
 * @author	Braden Harwood 	(bharwood@pdx.edu)
 */

#include <stdarg.h>
#include "./queueADT/mem_queue.h"
#include "./parser/parser.h"
#include "./dimm/dimm.h"
#include <limits.h>
#include "wrappers.h"
#include "./stats/stats.h"

// Parameters
#define CMD_QUEUE_SIZE 16
#define BANK_GROUPS 4
#define BANKS_PER_GROUP 4
#define ROWS_PER_BANK 32768

typedef struct{
	uint8_t			dimm_time;
	memCmd_t		dimm_op;
	uint8_t			grp_time[BANK_GROUPS];
	memCmd_t		grp_op[BANK_GROUPS];
	uint8_t			bank_time[BANK_GROUPS*BANKS_PER_GROUP];
	memCmd_t		bank_op[BANK_GROUPS*BANKS_PER_GROUP];
}dimmSchedule_t;

//Macro to extract absolute bank number from an inputCommandPtr_t
#define bank_number(X)	((X)->bankGroups*BANKS_PER_GROUP+(X)->banks)

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
void optimizedExecution(void);
unsigned indexOldestCmd(bool* touched, operation_t cmd);
unsigned indexHighestThrshld(bool* touched);
unsigned indexCmdWithOpenRow(bool* touched, operation_t cmd);
void inOrderExecution(void);
int sendMemCmd(inputCommandPtr_t command);
void updateAllRequests(bool *touched);
int updateCmdState(inputCommandPtr_t command);
bool processRequest(unsigned index, dimmSchedule_t *schedule);
uint8_t scheduleRequest(uint8_t timeTillCmd, inputCommandPtr_t request, dimmSchedule_t *schedule);
uint8_t reserveTime(int cmdTime, inputCommandPtr_t command, dimmSchedule_t *schedule);

#ifdef VERBOSE
void writeOutput(uint8_t delay, const char *format, ...);
void printOutput(void);
#endif

#ifdef DEBUG
void printSchedule(dimmSchedule_t *sched);
#endif

/*
 * Global variables
 */
unsigned long long currentTime; //Current simulation time
queuePtr_t commandQueue; //Queue for storing memory requests from input trace
queuePtr_t outputBuffer; //If running VERBOSE build, stores output messages
parser_t *parser; //Parses input file to create inputCommand_t structs
dimm_t *dimm; //The DIMM struct that the controller is sending DRAM cmds to
FILE *output_file; //Output stream if specified by the user. Defaults to stdout
bool optimizedFlag; //Enables the optimized scheduling algorithm
bool statFlag; //Enables printing statistics at the completion of the simulation

// Time in queue thresholds for each operation. Used with the -opt flag 
// scheduling.
uint16_t fchThrshld = 500;
uint16_t rdThrshld = 1000;
uint16_t wrThrshld = 2000;

int main(int argc, char **argv)
{
	initSim(argc, argv); // Init structures

	// Initialize global time variable
	currentTime = 0;

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

		//Send DRAM command if on an even CPU clock cycle
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
		age_queue(outputBuffer,UCHAR_MAX); 
		printOutput();
	}
	#endif
	
	if (statFlag)
		displayStats(output_file);

	// garbage collection to close fd's and clean the heap
	garbageCollection();

	return 0; // End
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
	clean_Stats();
	dimm_deinit(dimm);
	Fclose(output_file);
}

/**
 * @fn		initSim
 * @brief	Initializes all the global pointer in the simulation
 *
 * @param	argc	Number of arguments passed from the command line
 * @param	argv	Array of \0 terminated strings each containing one argument
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

	init_Stats();

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
		writeOutput(0, "%llu: Added new request to queue:\n Time:%llu Type:%6s Address:%#010llX, Group:%u, Bank:%u, Row:%5u, Upper Column:%3u", currentTime, currentCommandLine->cpuCycle, getCommandString(currentCommandLine->operation), currentCommandLine->address, currentCommandLine->bankGroups, currentCommandLine->banks, currentCommandLine->rows, currentCommandLine->upperColumns);
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
	// If the command queue is empty and the parser is at EOF, then the simulation is done
	if (is_empty(commandQueue) && parser->lineState == ENDOFFILE)
	{
		#ifdef DEBUG
		Printf("At time %llu, last command removed from queue. Ending simulation.\n", currentTime);
		#endif
		return 0;
	}
	
	unsigned long long timeJump = getTimeJump(); //Calculate time jump

	#ifdef DEBUG
	Printf("mem_sim.advanceTime(): Time jump will be %llu\n", timeJump);
	#endif
	// Check if the time jump would take us past the limit of the simulation
	if (currentTime + timeJump < currentTime)
	{
		Fprintf(stderr, "Simulation exceeded max simulation time of %llu. Ending simulation\n", ULLONG_MAX);
		currentTime = ULLONG_MAX;
		return 0;
	}
	// Advance current time by that calculated time
	currentTime += timeJump;
	#ifdef DEBUG
	Printf("mem_sim.advanceTime(): Aging command queue\n");
	#endif

	// Age items in queue by time advanced
	if (timeJump > (unsigned long long)UCHAR_MAX)
	{
		age_queue(commandQueue, UCHAR_MAX);
		#ifdef VERBOSE
		age_queue(outputBuffer, UCHAR_MAX);
		#endif
	}
	else
	{
		age_queue(commandQueue, timeJump);
		#ifdef VERBOSE
		age_queue(outputBuffer, timeJump);
		#endif
	}

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
	optimizedFlag = false;
	statFlag = false;

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
		if (argv[i][0] != '-')
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
		else if (!strcmp(argv[i], "-o")) // If the -o flag is asserted
		{
			if (i + 1 < argc) //makes sure we won't go out of bounds in strstr
			{
				if (strstr(argv[i + 1], ".txt") != NULL)  // Verify the user specified a .txt filename
				{
					
					outFile = argv[i + 1];
					output_file = Fopen(outFile, "w");
					i++;
					#ifdef DEBUG
					Printf("mem_sim.parseArgs():Output file flag detected. File Format Correct. Print to %s\n", outFile);
					#endif
				}
				else
				{	
					output_file = Fopen("output.txt", "w");
					Fprintf(stderr, "Warning in mem_sim.parseArgs(): Output file not a .txt file. Using 'output.txt'\n%s\n", argv[i+1]);
				}
			}
			else
			{
				
				output_file = Fopen("output.txt", "w");
				#ifdef DEBUG
				Printf("mem_sim.parseArgs():Output file flag detected. File name not provided. Default to 'output.txt'\n");
				#endif
			}
			out_flag = true; 
		}
		else if(!strcmp(argv[i], "-opt"))
		{
			optimizedFlag = true;
		}
		else if(!strcmp(argv[i], "-stat"))
		{
			statFlag = true;
		}
		else
		{
			Printf("Invalid argument: %s\n", argv[i]);
		}
	}
	// Check bounds on parameters and assert defaults and/or print error messages
	if (fileName == NULL)
	{
		Fprintf(stderr, "Error in mem_sim: No input file name provided.\n");
	}

	if(!out_flag)
	{	
		output_file = stdout;
		#ifdef DEBUG
		Printf("mem_sim.parseArgs():Output file flag NOT detected. Printing to stdout\n\n");
		#endif
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
	if (optimizedFlag)
		optimizedExecution();
	else
		inOrderExecution();
}

/**
 * @fn		optimizedExecution
 * @brief	Chose request, if any to advance with a DRAM command.
 *
 * @detail	Prioritize requests by:
 *		1. Request that's exceeded it's time in queue threshold by the largest value
 *		2. Fetches to open rows
 *		3. Reads to open rows
 *		4. Writes to open rows
 *		5. Oldest Fetch in the queue
 *		6. Oldest read in the queue
 *		7. Oldest write in the queue
 *
 *		Lower priority requests can have DRAM commands issued for them if the DRAM
 *		command will not impede any higher priority memory request.
 */
void optimizedExecution()
{
	//Setup a blank schedule
	dimmSchedule_t schedule;
	schedule.dimm_op = NONE;
	for (int i = 0; i < BANK_GROUPS; i++)
		schedule.grp_op[i] = NONE;
	for (int i = 0; i < BANK_GROUPS*BANKS_PER_GROUP; i++)
		schedule.bank_op[i] = NONE;

	bool touched[commandQueue->size]; //Used to track which requests have already been examined
	updateAllRequests(touched);
	
	//Process all requests that have exceeded their threshold time in queue. Largest excess time to smallest
	for (unsigned index = indexHighestThrshld(touched); index != 0; index = indexHighestThrshld(touched))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): High TIQ over threshold at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}

	//Process all fetch requests that target an already activated row
	for (unsigned index = indexCmdWithOpenRow(touched, IFETCH); index != 0; index = indexCmdWithOpenRow(touched, IFETCH))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): Fetch to open row at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}

	//Process all read requests that target an already activated row
	for (unsigned index = indexCmdWithOpenRow(touched, RD); index != 0; index = indexCmdWithOpenRow(touched, RD))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): Read to open row at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}

	//Process all write requests that target an already activated row
	for (unsigned index = indexCmdWithOpenRow(touched, WR); index != 0; index = indexCmdWithOpenRow(touched, WR))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): Write to open row at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}
	
	//Process all fetches, oldest to newest
	for (unsigned index = indexOldestCmd(touched, IFETCH); index != 0; index = indexOldestCmd(touched, IFETCH))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): Fetch at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}
	
	//Process all reads, oldest to newest
	for (unsigned index = indexOldestCmd(touched, RD); index != 0; index = indexOldestCmd(touched, RD))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): Read at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}
	
	//Process all writes, oldest to newest
	for (unsigned index = indexOldestCmd(touched, WR); index != 0; index = indexOldestCmd(touched, WR))
	{
		#ifdef DEBUG
		Printf("mem_sim.optimizedExecution(): Write at index %u\n", index);
		#endif
		if (processRequest(index, &schedule))
			return; //If process request returns true, then a DRAM cmd has been sent
		
		touched[index-1] = true; //Otherwise, update this index as processed and continue scheduling
	}

	//Make sure no commands were missed
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		if (!touched[i-1])
			Fprintf(stderr, "Warning in mem_sim.optimizedExecution(): Reached end of priority list without touching index %u\n", i);
	}
}

/**
 * @fn		indexOldestCmd
 * @brief	Return the oldest request of a specific type from the untouched requests
 *
 * @detail	Scans through all untouched requests in the queue indicated by a value of
 *		false at that request's index-1 in the provided boolean array. Finds the
 *		oldest request that matches the specified operation and returns the index
 *		to that request in the queue.
 * @param	touched	Array tracking which indexes have already been analyzed. A value
 *			of true at a specific index mean that the request in the queue at
 *			index + 1 can be ignored by this function
 * @param	cmd	The specific operation being scanned for. IFETCH, RD, or WR.
 * @return	The index of the oldest, unanalyzed operation in the queue. 0 if none
 *		exist.
 */
unsigned indexOldestCmd(bool* touched, operation_t cmd)
{
	inputCommandPtr_t current;
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		if (touched[i-1])
			continue;

		current = peakCommand(i);
		if (current->operation == cmd)
			return i;
	}
	return 0;
}

/**
 * @fn		indexCmdWithOpenRow
 * @brief	Finds the index of the oldest specified operation in the queue that targets an open row
 *
 * @detail	Scans through all untouched requests in the queue (indicated by a value of
 *		false at that request's index-1 in the provided boolean array). Finds the
 *		oldest request that matches the specified operation and targets an open row
 *		in the DIMM. Returns the index to that request in the queue.
 * @param	touched	Array tracking which indexes have already been analyzed. A value
 *			of true at a specific index mean that the request in the queue at
 *			index + 1 can be ignored by this function
 * @param	cmd	The specific operation being scanned for. IFETCH, RD, or WR.
 * @return	The index of the oldest, unanalyzed operation in the queue that targets an open row. 
 *		0 if none exist.
 */
unsigned indexCmdWithOpenRow(bool* touched, operation_t cmd)
{
	inputCommandPtr_t current;
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		if (touched[i-1])
			continue;

		current = peakCommand(i);
		if (current->operation == cmd && dimm_rowOpen(dimm, current->bankGroups, current->banks, current->rows))
			return i;
	}
	return 0;
}

/**
 * @fn		indexHighestThrshld
 * @brief	Returns index in command queue of request that is farthest past its threshold
 *
 * @detail	The provided array of bools will determine which requests in the queue are considered.
 *		Values of true in the bool array will cause the same index in the request queue to be
 *		ignored when examining the requests to determine the one that is farthest past its
 *		time in queue threshold.
 * @param	touched Pointer to array of bools that mark which commands have already been touched.
 * @return	Index of request that's farthest past its time in queue threshold. 0 if no requests
 *		are past their threshold
 */
unsigned indexHighestThrshld(bool* touched)
{
	unsigned index = 0; //Track index farthest past threshold
	uint16_t maxExcess = 0; //Track largest excess past threshold

	inputCommandPtr_t current;
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		//Ignore requests that have already been touched
		if (touched[i-1])
			continue;
		
		current = peakCommand(i);
		uint16_t tiq = getTimeInQueue(i, commandQueue);	//Time in queue
		uint16_t threshold;
		switch(current->operation)
		{
			case IFETCH:
			threshold = fchThrshld;
			break;
			case RD:
			threshold = rdThrshld;
			break;
			case WR:
			threshold = wrThrshld;
			break;
		}
		if (tiq < threshold)
			continue;

		if (tiq - threshold > maxExcess)
		{
			maxExcess = tiq - threshold;
			index = i;
		}
	}
	return index;
}

/**
 * @fn		inOrderExecution
 * @brief	Sends command to DIMM with a schedule for loose in-order execution
 *
 * @detail	This algorithm will prioritize the oldest requests but it will allow newer
 *		requests to be serviced but only if doing so does not slow down the time to
 *		satisfy older requests
 */
void inOrderExecution()
{
	//Setup the scheduler
	dimmSchedule_t schedule;
	schedule.dimm_op = NONE;
	for (int i = 0; i < BANK_GROUPS; i++)
		schedule.grp_op[i] = NONE;
	for (int i = 0; i < BANK_GROUPS*BANKS_PER_GROUP; i++)
		schedule.bank_op[i] = NONE;
	
	//Update all requests in the queue
	bool touched[commandQueue->size];
	updateAllRequests(touched);

	//Start at the top of the queue and go down, giving priority to the oldest requests
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		if (!touched[i-1]) //Skip requests that are just waiting for data
			if (processRequest(i, &schedule)) // If true is returned, then DIMM cmd was sent
				return;
	}
}

/**
 * @fn		processRequest
 * @brief	Attempts to execute the request at the given index. Schedules it if not executed
 *
 * @detail	Checks with the provided schedule if the request at the given index in the queue
 *		can be executed without getting in the way of any higher priority requests. If
 *		a dimm command can't be sent, then this request is added to the schedule to
 *		prevent lower priority requests from getting in its way and its age is updated
 *		with the expected cycles till a DRAM cmd can be issued to service this request
 * @param	index	Index in the queue of the request of interest
 * @param	schedule	Pointer to dimmSchedule_t that will be holding schedule information
 *				of all higher priority request.
 * @return	True if a DRAM command is issued. False otherwise.
 */
bool processRequest(unsigned index, dimmSchedule_t *schedule)
{
	inputCommandPtr_t request = peakCommand(index);
	uint8_t timeTillCmd = getAge(index, commandQueue);
	#ifdef DEBUG
	Printf("mem_sim.processRequest():At index %u: Age:%u nextCmd:%s Time:%llu Type:%6s Group:%u, Bank:%u, Row:%u, Upper Column:%u\n", index, timeTillCmd, nextCmdToString(request->nextCmd), request->cpuCycle, getCommandString(request->operation), request->bankGroups, request->banks, request->rows, request->upperColumns);
	#endif
	timeTillCmd = scheduleRequest(timeTillCmd, request, schedule);
	if (timeTillCmd == 0)
	{
		setAge(index, sendMemCmd(request), commandQueue);
		return true;
	}
	setAge(index, timeTillCmd, commandQueue);
	return false;
}

/**
 * @fn		scheduleRequest
 * @brief	Checks if a request can have a DIMM cmd issued without slowing any higher priority requests
 *
 * @detail	Uses entries in the schedule to examine the next time the DIMM, target group, and target bank
 *		to see if there is nothing scheduled or, if there is, can a DRAM cmd be issued to progress
 *		this request without slowing down the higher priority request that's already on the schedule
 * @param	timeTillCmd	CPU cycles until this request will be ready for its next DRAM command
 * @param	request	The request being scheduled
 * @param	schedule	Pointer to the schedule struct
 * @return	Estimated time till the next DRAM cmd can be issued for this request without distrupting
 *		higher priority requests.
 */
uint8_t scheduleRequest(uint8_t timeTillCmd, inputCommandPtr_t request, dimmSchedule_t *schedule)
{
	if (timeTillCmd > 0)
	{
		return reserveTime(timeTillCmd, request, schedule);
	}
	
	if (dimm_recoveryTime(request->nextCmd, schedule->dimm_op) > schedule->dimm_time)
	{
		#ifdef DEBUG
		Printf("mem_sim.scheduleRequest():Blocked in dimm\n");
		#endif
		return reserveTime(schedule->dimm_time + dimm_recoveryTime(schedule->dimm_op, request->nextCmd), request, schedule); 
	}
	if (group_recoveryTime(request->nextCmd, schedule->grp_op[request->bankGroups]) > schedule->grp_time[request->bankGroups])
	{
		#ifdef DEBUG
		Printf("mem_sim.scheduleRequest():Blocked in bank group.\n");
		#endif
		return reserveTime(schedule->grp_time[request->bankGroups]+group_recoveryTime(schedule->grp_op[request->bankGroups], request->nextCmd), request, schedule);
	}
	if (bank_recoveryTime(request->nextCmd, schedule->bank_op[bank_number(request)]) > schedule->bank_time[bank_number(request)])
	{
		#ifdef DEBUG
		Printf("mem_sim.scheduleRequest():Blocked in bank.\n");
		#endif
		return reserveTime(schedule->bank_time[bank_number(request)]+bank_recoveryTime(schedule->bank_op[bank_number(request)], request->nextCmd), request, schedule);
	}
	return 0;
}

/**
 * @fn		reserveTime
 * @brief	Reserves time in the schedule for a priority request
 *
 * @detail	Reserves time separately in the dimm, target group, and target bank for this request. Reserving time
 *		in the schedule ensures that other, lower priority requests, won't issue a DRAM command if it would
 *		cause this request's scheduled time in the DIMM/group/bank to be delayed.
 * @param	cmdTime	Time this request will be ready for its next DRAM command
 * @param	command	Pointer to the request being scheduled
 * @param	schedule	Pointer to the schedule struct
 * @return	Estimated time in CPU clock cycles until a DRAM can be issued for this request.
 */
uint8_t reserveTime(int cmdTime, inputCommandPtr_t command,dimmSchedule_t *schedule)
{
	if (schedule->bank_op[bank_number(command)] == NONE ||
		(cmdTime + bank_recoveryTime(command->nextCmd, schedule->bank_op[bank_number(command)]) <= schedule->bank_time[bank_number(command)] ))
	{
		schedule->bank_op[bank_number(command)] = command->nextCmd;
		schedule->bank_time[bank_number(command)] = cmdTime;
		#ifdef DEBUG
		Printf("mem_sim.inOrderExecution():Reserved bank.\n");
		#endif
	}
	else
	{
		#ifdef DEBUG
		printSchedule(schedule);
		#endif
		return schedule->bank_time[bank_number(command)] + SCALE_FACTOR;
	}

	if (schedule->grp_op[command->bankGroups] == NONE ||
		(cmdTime + group_recoveryTime(command->nextCmd, schedule->grp_op[command->bankGroups]) <= schedule->grp_time[command->bankGroups]))
	{
		schedule->grp_op[command->bankGroups] = command->nextCmd;
		schedule->grp_time[command->bankGroups] = cmdTime;
		#ifdef DEBUG
		Printf("mem_sim.inOrderExecution():Reserved Group.\n");
		#endif
	}
	else
	{
		#ifdef DEBUG
		printSchedule(schedule);
		#endif
		return schedule->grp_time[command->bankGroups] + SCALE_FACTOR;
	}

	if (schedule->dimm_op == NONE ||
		(cmdTime + dimm_recoveryTime(command->nextCmd, schedule->dimm_op) <= schedule->dimm_time))
	{
		schedule->dimm_op = command->nextCmd;
		schedule->dimm_time = cmdTime;
		#ifdef DEBUG
		Printf("mem_sim.inOrderExecution():Reserved dimm.\n");
		#endif
	}
	else
	{
		#ifdef DEBUG
		printSchedule(schedule);
		#endif
		return schedule->dimm_time + SCALE_FACTOR;
	}
	#ifdef DEBUG
	printSchedule(schedule);
	#endif
	return cmdTime;
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
		case WRITE:
			command->nextCmd = REMOVE;
			retVal = dimm_write(dimm, command->bankGroups, command->banks, command->rows, currentTime);
			#ifdef VERBOSE
			writeOutput(0, "%llu: Group %u, Bank %u received write command to row %u, upper column %u", currentTime, command->bankGroups, command->banks, command->rows, command->upperColumns);
			writeOutput(retVal-(TBURST*SCALE_FACTOR), "%llu: Group %u, Bank %u has begun latching data and storing in row %u, upper column %u", currentTime+retVal-(TBURST*SCALE_FACTOR), command->bankGroups, command->banks, command->rows, command->upperColumns);
			writeOutput(retVal, "%llu: Group %u, Bank %u has completed writing to row %u, upper column %u", currentTime+retVal, command->bankGroups, command->banks, command->rows, command->upperColumns);
			#else
			Fprintf(output_file, "%'26llu\tWR  %X %X %X\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
			#endif
			break;
		case READ:
			command->nextCmd = REMOVE;
			retVal = dimm_read(dimm, command->bankGroups, command->banks, command->rows, currentTime);
			#ifdef VERBOSE
			writeOutput(0, "%llu: Group %u, Bank %u received read command to row %u, upper column %u", currentTime, command->bankGroups, command->banks, command->rows, command->upperColumns);
			writeOutput(retVal-(TBURST*SCALE_FACTOR), "%llu: Group %u, Bank %u has begun bursting data from row %u, upper column %u", currentTime+retVal-(TBURST*SCALE_FACTOR), command->bankGroups, command->banks, command->rows, command->upperColumns);
			writeOutput(retVal, "%llu: Group %u, Bank %u has completed burst from row %u, upper column %u", currentTime+retVal, command->bankGroups, command->banks, command->rows, command->upperColumns);
			#else
			Fprintf(output_file, "%'26llu\tRD  %X %X %X\n", currentTime, command->bankGroups, command->banks, (((unsigned long)command->upperColumns)<<3) + command->lowerColumns);
			#endif
			break;
		case ACTIVATE:
			if (command->operation == WR)
				command->nextCmd = WRITE;
			else
				command->nextCmd = READ;
			retVal = dimm_activate(dimm, command->bankGroups, command->banks, command->rows, currentTime);
			#ifdef VERBOSE
			writeOutput(0, "%llu: Group %u, Bank %u has begun activating row %u", currentTime, command->bankGroups, command->banks, command->rows);
			writeOutput(retVal, "%llu: Group %u, Bank %u has completed activating row %u", currentTime+retVal, command->bankGroups, command->banks, command->rows);
			#else
			Fprintf(output_file, "%'26llu\tACT %X %X %X\n", currentTime, command->bankGroups, command->banks, command->rows);
			#endif
			break;
		case PRECHARGE:
			command->nextCmd = ACTIVATE;
			retVal = dimm_precharge(dimm, command->bankGroups, command->banks, currentTime);
			#ifdef VERBOSE
			writeOutput(0, "%llu: Group %u, Bank %u has closed row %u and begun precharging", currentTime, command->bankGroups, command->banks, dimm->group[command->bankGroups]->bank[command->banks]->row);
			writeOutput(retVal, "%llu: Group %u, Bank %u has completed precharging and is ready to activate a row", currentTime+retVal, command->bankGroups, command->banks);
			#else
			Fprintf(output_file, "%'26llu\tPRE %X %X\n", currentTime, command->bankGroups, command->banks);
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
 * @fn		updateAllRequests
 * @brief	Updates the state of all requests in the queue
 *
 * @detail	Will update the state of all requests and setup the provided boolean array.
 *		Requests that still require at least one DRAM cmd will have their index in
 *		the array initialized to false. Requests that are just waiting for their data
 *		to finish will have their index in the array initialized to true.
 * @param	touched	Array tracking which requests have been examined. All requests that
 */
void updateAllRequests(bool *touched)
{
	inputCommandPtr_t current;
	for (unsigned i = 1; i <= commandQueue->size; i++)
	{
		current = peakCommand(i);
		if (current->nextCmd == REMOVE)
		{
			if (getAge(i, commandQueue) == 0)
			{
				#ifdef VERBOSE
				writeOutput(0, "%llu: Completed request from Time:%llu Type:%6s Group:%u, Bank:%u, Row:%u, Upper Column:%u", currentTime, current->cpuCycle, getCommandString(current->operation), current->bankGroups, current->banks, current->rows, current->upperColumns);
				#endif
				if (statFlag)
				{
					request_t *info = Malloc(sizeof(request_t));
					info->timeInQueue = getTimeInQueue(i, commandQueue);
					info->type = current->operation;
					addRequest(info);
				}
				free(queueRemove(commandQueue,i));
				i--;
				continue;
			}
			touched[i-1] = true;
			continue;
		}
		setAge(i, updateCmdState(current), commandQueue);
		touched[i-1] = false;
	}
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
	int timeTillCmd = (command->operation == WR) ?	
		dimm_canWrite(dimm, command->bankGroups, command->banks, command->rows, currentTime) :
		dimm_canRead(dimm, command->bankGroups, command->banks, command->rows, currentTime);
	if (timeTillCmd >= 0)
	{
		command->nextCmd = (command->operation == WR) ? WRITE : READ;
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
void writeOutput(uint8_t delay, const char *format, ...)
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
	while (getAge(1, outputBuffer) == 0 && !is_empty(outputBuffer))
	{
		char *output = (char *)queueRemove(outputBuffer, 1);
		Fprintf(output_file, "%s\n", output);
		free(output);
	}
}
#endif

#ifdef DEBUG
void printSchedule(dimmSchedule_t *sched)
{
	Printf("----------Cmd Schedule----------\n");
	Printf("|-DIMM: %9s in %u\n", nextCmdToString(sched->dimm_op), sched->dimm_time);
	Printf("|-------------------------------\n");
	for (int i = 0; i < BANK_GROUPS; i++)
	{
		Printf("|---Group%2u: %9s in %u\n", i, nextCmdToString(sched->grp_op[i]), sched->grp_time[i]);
		for (int j = 0; j < BANKS_PER_GROUP; j++)
		{
			Printf("|-----Bank%2u: %9s in %u\n", j, nextCmdToString(sched->bank_op[i*BANKS_PER_GROUP+j]), sched->bank_time[i*BANKS_PER_GROUP+j]);
		}
		Printf("|-------------------------------\n");
	}
	Printf("--------------------------------\n");
}
#endif
