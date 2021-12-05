/**
 * @file  parser.c
 * @brief File input parser source code for DRAM memory controller simulation
 *
 * @detail 	This parser is used to parse through memory trace input files for the
 * 		memory controller simulation. It accepts input in the form
 *		<time> <command> <address in hex> and then maps this address to
 *		the address's row, column, bank, bank group, and byte select fields.
 *		The parser gathers input one line at a time and holds it until the
 *		cpu time has passed and there is room in the queue for it. It also
 * 		informs the simulator when it reaches the end of the file.
 *		ECE 485/585 Final Project, Dr. Mark Faust
 *		Portland State University, Fall 2021
 *
 * @date	Presented December 6th, 2021
 *
 * @author	Stephen Short 	(steshort@pdx.edu)
 * @author	Drew Seidel	(dseidel@pdx.edu)
 * @author	Michael Weston 	(miweston@pdx.edu)
 * @author	Braden Harwood 	(bharwood@pdx.edu)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "parser.h"
#include "../wrappers.h"

// Helper Functions:
void printCurrentLine(inputCommandPtr_t currentCommandLine);


parserPtr_t initParser(char* inputFile)
{
	// Allocate space for the parser and initialize the state, open file, and complete first read.
	parserPtr_t newParser;
	newParser = Malloc(sizeof(parser_t));
		
	// Open file pointer
	newParser->filename = Fopen(inputFile, "r");
	
	newParser->lineState = PARSE_ERROR;
	newParser->nextLine = NULL;
	
	// Grab first line and update state accordingly
	if (prepCommand(newParser) == -1)
	{
		cleanParser(newParser);
		return NULL;
	}
	
	#ifdef DEBUG
		Printf("parser: Initialized new parser. Parser state:%s\n",getParserState(newParser->lineState));
	#endif

	return newParser; // Return the new parser ADT
}

void cleanParser(parser_t* parser)
{
	if (parser == NULL)
		return;

	Fclose(parser->filename);
	free(parser->nextLine);
	free(parser);
}

inputCommandPtr_t getCommand(parserPtr_t parser, unsigned long long currentTime)
{
	if (parser->nextLine == NULL)
		return NULL;

		#ifdef DEBUG
			Printf("Parser: Next command at time %llu. Current time %llu\n", parser->nextLine->cpuCycle, currentTime);
		#endif
	if (parser->nextLine->cpuCycle <= currentTime)
	{
		#ifdef DEBUG
			Printf("Parser: Returning command. Then parsing new command.\n");
		#endif
		inputCommandPtr_t newCommand = parser->nextLine;
		if(prepCommand(parser) != 0)
		{
			Fprintf(stderr, "Error in Parser: Failed to parse next line\n");
			exit(EXIT_FAILURE);
		}
		return newCommand;
	}
	#ifdef DEBUG
		Printf("Parser: Holding on to command and returning NULL.\n");
	#endif
	return NULL;
}

int prepCommand(parserPtr_t parser)
{
	#ifdef DEBUG
		Printf("Parser: Parsing new line\n");
	#endif
	char inputLine[128];
		
	if (fgets(inputLine, 128, parser->filename) == NULL)
	{ // Returns ENDOFFILE if it reaches the end of the file
		#ifdef DEBUG
			Printf("Parser: Encountered end of file.\n");
		#endif
		parser->lineState = ENDOFFILE;
		parser->nextLine = NULL;
		return 0;
	}

	#ifdef DEBUG
		Printf("Parser read in:%s\n",inputLine);
	#endif
	
	// Not end of file
	int numFields;
	
	unsigned commandInt;
	unsigned long long time;
	unsigned long long address;
	numFields = sscanf(inputLine, " %llu %d %llx\n", &time, &commandInt, &address);
	
	if (numFields != 3)
	{
		Fprintf(stderr,"Error in Parser.prepCommand(): incorrect number of fields parsed from file.\nThis line: ");
		Fprintf(stderr,"%s", inputLine);
		return -1;
	}

	// Check if address is out of range, trying to reference a row that is impossible to reach.
	if ((address >> 18) > 0x7FFF)	
	{
		Fprintf(stderr,"Error in Parser.prepCommand(): Address provided is out of range for the size of memory. ");
		Fprintf(stderr,"Refereneces invalid row address.\nThis line: ");
		Fprintf(stderr,"%s", inputLine);
		return -1;
	}
	
	// Allocate memory for next line
	parser->nextLine = Malloc(sizeof(inputCommand_t));

	parser->nextLine->cpuCycle = time;
	parser->nextLine->operation = (operation_t)commandInt;
	parser->nextLine->nextCmd = ((operation_t)commandInt == WR) ? WRITE : READ;
	parser->nextLine->address = address;
	parser->nextLine->rows = address >> 18;
	parser->nextLine->upperColumns = (address & 0x3FC00) >> 10;
	parser->nextLine->banks = (address & 0x300) >> 8;
	parser->nextLine->bankGroups = (address & 0xC0) >> 6;
	parser->nextLine->lowerColumns = (address & 0x38) >> 3;
	parser->nextLine->byteSelect = (address & 0x7);
	parser->nextLineTime = time;
	parser->lineState = READY;
	#ifdef DEBUG
		printCurrentLine(parser->nextLine);
	#endif
	return 0;
}

void printCurrentLine(inputCommandPtr_t currentCommandLine)
{
	Printf("PARSER, Completed Parsing Line: Time = %10llu, Command Attempt = %6s, Address = 0x%010llX, Row = %5u, Upper Column = %3u, Bank = %2u, Bank Group = %1u, Lower Column = %1u, Byte Select = %1u\n", currentCommandLine->cpuCycle, getCommandString(currentCommandLine->operation), currentCommandLine->address, currentCommandLine->rows, currentCommandLine->upperColumns, currentCommandLine->banks, currentCommandLine->bankGroups, currentCommandLine->lowerColumns, currentCommandLine->byteSelect); 
}

const char* getCommandString(operation_t command)
{
   switch (command) 
   {
	case RD: return "READ";
	case WR: return "WRITE";
	case IFETCH: return "IFETCH";
   }
   return "ERROR";
}

const char* nextCmdToString(memCmd_t cmd)
{
	switch(cmd)
	{
		case READ: return "READ";
		case WRITE: return "WRITE";
		case ACTIVATE: return "ACTIVATE";
		case PRECHARGE: return "PRECHARGE";
		case NONE: return "NONE";
		case REMOVE: return "REMOVE";
	}
	return "nextCmd NOT SET";
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
	return "ERROR";
}
