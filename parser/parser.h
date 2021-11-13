/*
MemoryController.h
*/
#ifndef _PARSER_H
#define _PARSER_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "./../queueADT/mem_queue.h"

/**
*	parser_state_t is used to interact with the parser ADT and understand what it currently holds and 
*		whether it returned a valid command line.
* States:
*	PARSE_ERROR - There was an error in parsing a line, the function can be called again until there 
*		is no error. Helpful to parse through a bad file without ending the program. Also used to skip
*		over lines that are invalid.
*	FUTURE		- The command line is stored in the parser adt, but it isn't supposed to be added to the 
*		queue until some future time. Request the information from the parser adt on when the future time is.
*		No new line will be grabbed until after that future time.
*	VALID		- The command stored in the ADT is valid for the current time. The newCommand pointer has been
*		updated to point to the allocated memory for the command line. It is ready to be added to the queue.
*	READY		- The current line in the parser ADT is old and it is time to get another line from the file
*		and update the status.
*	ENDOFFILE 	- There are not more lines to get. We are at the end of the file.
*/
typedef enum _parser_e {PARSE_ERROR = -1, FUTURE, VALID, READY, ENDOFFILE} parser_state_t;

/**
*	parser_t is the ADT that holds the file pointer, when the current command is ready to be sent to the queue
*	(cpu cycle time), a pointer to the command line if there is a future line currently stored, and the state
*	of the command line currently being held in the parser or the state of the parser if there is no valid line
*	currently held in the parser ADT.
*/
typedef struct _parser_s 
{
	FILE *				filename;
	unsigned long long	nextLineTime;
	inputCommandPtr_t	nextLine;
	parser_state_t		lineState;
} parser_t, *parserPtr_t;

// Function Definitions
/**
* FUNCTION:			initParser
*
* INFO:				Initiates a parser ADT that stores the input filename, current line from the 
*					file, the time that the current line should be added to the queue, and the state
*					of the line, which defines whether it is ready to have a new line read, should be
*					added at a future time, whether there was a parsing error, whether it has reached
*					the end of the file, and if the state of the line is valid which is used
*					in the getLine function to indicate to the simulator that the command pointer has
*					been updated with an allocated command line.
*
* PARAMS:			char* inputFile (input file name)
*
* RETURNS:			parserPtr_t (pointer to newly allocated parser ADT)
*/
parserPtr_t initParser(char* inputFile);

/**
* FUNCTION:			getLine
*
* INFO:				Depending on the current state of the parser, it will either return the existing command
					line if the current time is >= to the command's cpuCycle time or it will get a new line
					if it hasn't already reached the end of the file or if a parsing error has occurred.
					If the current state is a future time, this state is double checked to ensure that this 
					time hasn't been reached yet. If it has reached or passed that time, it will update the 
					pointer passed to it with the command line and indicate that it is valid. This command
					should only be used when you are ready to get a line and add it to the queue. ***If you are
					not ready for a new command, do not run this function.***
*
* PARAMS:			parserPtr_t parser (pointer to parser adt)
					inputCommandPtr_t newCommand (pointer provided by the simulator that will be used to pass on
						the allocated memory for the new command to the queue, it is then the queue's responsibility
						to free the memory when it is removed)
					unsigned long long currentTime (current CPU cycle time)
*
* RETURNS:			parser_state_t (enum which holds the state of the returned information)
					States:
					PARSE_ERROR - There was an error in parsing a line, the function can be called again until there 
						is no error. Helpful to parse through a bad file without ending the program. Also used to skip
						over lines that are invalid.
					FUTURE		- The command line is stored in the parser adt, but it isn't supposed to be added to the 
						queue until some future time. Request the information from the parser adt on when the future time is.
						No new line will be grabbed until after that future time.
					VALID		- The command stored in the ADT is valid for the current time. The newCommand pointer has been
						updated to point to the allocated memory for the command line. It is ready to be added to the queue.
					READY		- The current line in the parser ADT is old and it is time to get another line from the file
						and update the status.
					ENDOFFILE 	- There are not more lines to get. We are at the end of the file.
					
*/
parser_state_t getLine(parserPtr_t parser, inputCommandPtr_t newCommand, unsigned long long currentTime);


#endif