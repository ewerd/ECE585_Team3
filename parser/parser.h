/*
MemoryController.h
*/
#ifndef _PARSER_H
#define _PARSER_H


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../queueADT/mem_queue.h"

// Function Definitions
bool debugMode();
FILE *openFile();
const char* getCommand(operation_t command);
bool readLine(FILE *inputFile, inputCommandPtr_t currentLine);

#endif