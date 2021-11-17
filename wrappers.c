/**
 * @file	wrappers.c
 * @brief	Simple wrapper functions to catch errors from system calls
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void Printf(char* format, ...)
{
	va_list args;
	va_start (args, format);
	int result = vprintf(format, args);
	va_end(args);

	if (result < 0) //If the print was not successful, alert user via stderr, collect garbage, and exit
	{
		perror("Error calling printf()");
		exit(EXIT_FAILURE);
	}
}

void Fprintf(FILE* stream, char* format, ...)
{
	va_list args;
	va_start (args, format);
	int result = vfprintf(stream, format, args);
	va_end(args);

	if (result < 0) //If the print was not successful, alert user via stderr and exit
	{
		perror("Error calling fprintf()");
		exit(EXIT_FAILURE);
	}
}

void *Malloc(size_t size)
{
	void* newPtr = malloc(size);
	if (newPtr == NULL)
	{
		perror("Error calling malloc()");
		exit(EXIT_FAILURE);
	}
	return newPtr;
}
