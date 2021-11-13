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
#include "parser.h"

// Helper Functions:
void printCurrentLine(inputCommandPtr_t currentCommandLine);


parserPtr_t initParser(char* inputFile)
{
	// Allocate space for the parser and initialize the state, open file, and complete first read.
	parserPtr_t newParser;
	if ((newParser = malloc(sizeof(parser_t))) == NULL)
	{
		fprintf(stderr, "\nError initParser: could not allocate space for parser.\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		// Open file pointer
		newParser->filename = fopen(inputFile, "r");
		if (inputFile == NULL)
		{
			fprintf(stderr, "\nError initParser: could not open file named %s\n", inputFile);
			exit(EXIT_FAILURE);
		}
		
		newParser->lineState = READY; // Nothing currently read yet
		
		// Grab first line and update state accordingly
		getLine(newParser, newParser->nextLine, 0);
		
		return newParser; // Return the new parser ADT
	}

}

parser_state_t getLine(parserPtr_t parser, inputCommandPtr_t newCommand, unsigned long long currentTime)
{
	// No more lines
	if (parser->lineState == ENDOFFILE)
	{
		return (parser_state_t) ENDOFFILE;
	}
	else if (parser->lineState == FUTURE)
	{ // Line currently stored in parser, but for a future time.
		// Check to see if it is no longer a future line
		if (parser->nextLineTime <= currentTime)
		{
			parser->lineState = READY;
			newCommand = parser->nextLine;
			return (parser_state_t) VALID;
		}
		else {
			return (parser_state_t) FUTURE;
		}
	}
	else if ((parser->lineState == READY) || (parser->lineState == PARSE_ERROR))
	{
		// Already read in the line, or there was an error reading in another line, read in another line
		// Simulator has received the previous line
		char inputLine[128];
		
		if (fgets(inputLine, 128, parser->filename) == NULL)
		{ // Returns ENDOFFILE if it reaches the end of the file
			return (parser_state_t) ENDOFFILE;
		}
		else 
		{
			// Not end of file
			
			int numFields;
			unsigned commandInt;
			inputCommandPtr_t nextCommand;
			
			// Allocate memory for next line
			if ((nextCommand = malloc(sizeof(inputCommand_t))) == NULL)
			{
				fprintf(stderr, "\nError getLine: could not allocate space for nextCommand.\n");
				exit(EXIT_FAILURE);
			}
			
			
			numFields = sscanf(inputLine, " %llu %d %llx\n", &(nextCommand->cpuCycle), &commandInt, &(nextCommand->address));
			
			
			if (numFields != 3)
			{
				fprintf(stderr,"Error, incorrect number of fields parsed from file.\nThis line: ");
				fprintf(stderr,"%s", inputLine);
				return (parser_state_t) PARSE_ERROR;
			}
			
			nextCommand->command		= (operation_t) commandInt;
			nextCommand->rows 			= (nextCommand->address & 0x1FFFC0000) >> 18;
			nextCommand->upperColumns 	= (nextCommand->address & 0x3FC00) >> 10;
			nextCommand->banks 			= (nextCommand->address & 0x300) >> 8;
			nextCommand->bankGroups		= (nextCommand->address & 0xC0) >> 6;
			nextCommand->lowerColumns	= (nextCommand->address & 0x38) >> 3;
			nextCommand->byteSelect 	=  nextCommand->address & 0x7;
			
			#ifdef DEBUG
				printCurrentLine(nextCommand);
			#endif
			
			if (nextCommand->cpuCycle > currentTime)
			{
				// Command is for the future
				parser->nextLine 		= nextCommand;
				parser->nextLineTime 	= nextCommand->cpuCycle;
				parser->lineState 		= FUTURE;
				return (parser_state_t) FUTURE;
			}
			else
			{
				// Command is valid now, tell simulator and return the command line
				parser->nextLineTime 	= nextCommand->cpuCycle;
				newCommand 				= nextCommand;
				parser->lineState 		= READY;
				return (parser_state_t) VALID;
			}
		}
	}
	
	// If you've made it this far, something has gone wrong
	fprintf(stderr, "Error: Reached end of getLine function. Should not be possible.\n");
	return (parser_state_t) PARSE_ERROR;
}

void printCurrentLine(inputCommandPtr_t currentCommandLine)
{
	printf("PARSER, Completed Parsing Line: Time = %10llu, Command Attempt = %6s, Address = 0x%010llX, Row = %5u, Upper Column = %3u, Bank = %2u, Bank Group = %1u, Lower Column = %1u, Byte Select = %1u\n", currentCommandLine->cpuCycle, getCommand(currentCommandLine->command), currentCommandLine->address, currentCommandLine->rows, currentCommandLine->upperColumns, currentCommandLine->banks, currentCommandLine->bankGroups, currentCommandLine->lowerColumns, currentCommandLine->byteSelect); 
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

const char* getParserState(parser_state_t state)
{
	switch(state)
	{
		case PARSE_ERROR: return "PARSE_ERROR";
		case FUTURE: return "FUTURE";
		case VALID: return "VALID";
		case READY: return "READY";
		case ENDOFFILE: return "ENDOFFILE";
	}
}