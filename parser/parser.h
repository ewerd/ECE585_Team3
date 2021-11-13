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

typedef enum _parser_e {PARSE_ERROR = -1, FUTURE, VALID, READY, ENDOFFILE} parser_state_t;

// define row from file separation
typedef struct _parser_s 
{
	FILE *				filename;
	unsigned long long	nextLineTime;
	inputCommandPtr_t	nextLine;
	parser_state_t		lineState;
} parser_t, *parserPtr_t;

// Function Definitions
parser_state_t getLine(parserPtr_t parser, inputCommandPtr_t newCommand, unsigned long long currentTime);
parserPtr_t initParser(char* inputFile);

#endif