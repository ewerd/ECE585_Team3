/**
 * @file	writeTrace.c
 * @brief	Helpful program for writing trace files
 *
 * @flags	-step	Time step between each request
 *		-mult	Number of requests to put into one time slot
 *		-end	Time to end trace
 *
 * @date	11/5/21
 * @author	Stephen Short(steshort@pdx.edu)
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * Helper function declarations
 */
void Printf(char* format, ...);
void Fprintf(FILE* stream, char* format, ...);
uint8_t isNumber(const char* test);
void expectedNumber(char* flag, char* arg);
void parseFlags(int argc, char** argv);
void Fclose(FILE *stream);
FILE* openFile(void);
unsigned int getInstruction(unsigned long long time);
unsigned long long getAddr(unsigned long long time);
void createTrace(FILE* trace);

unsigned long long timestep = 1;
unsigned long long endtime = 1;
int multipleEntries = 1;
unsigned long long address = 0x000000000; 
long int address_count = 0;

int main(int argc, char** argv)
{
	//Process command line args
	parseFlags(argc, argv);
	//Open/create file
	FILE* trace = openFile();
	//Write entries
	createTrace(trace);
	//Close file
	Fclose(trace);

	return 0;
}

/**
 * @fn		createTrace
 * @brief	Fills the trace file with memory references
 *
 * @param	trace	fd of the trace file being constructed.
 */
void createTrace(FILE* trace)
{
	long long time = 0;
	while (time <= endtime)
	{
		for (int i = 0; i < multipleEntries; i++)
		{
			Fprintf(trace, "%llu %u 0X%09llX\n", time, getInstruction(time), getAddr(time));
		}
		if (time == endtime)
			break;
		time += timestep;
	}
}

/**
 * @fn		getAddr
 * @brief	Returns the address of the request at the current time
 *
 * @details	We can flesh this out later to either generate an address as a function of time or randomly
 * @param	time	Current time
 * @return	A memory address
 */
unsigned long long getAddr(unsigned long long time) 
{
	
	//could automate with flag to create types of test cases
	if (time %2 == 0)
	address =  0x000000000;
	else 
	address =  0X000080000;
	//
	//address =  0x000000000 + (address_count << 18); //succsessive rows


	address_count++; 
	if (address_count == 32768)
		address_count = 0; 
		
	return address; 
}

/**
 * @fn		getInstruction
 * @brief	Returns the instruction of the request at the current time
 *
 * @details	We can flesh this out later to either generate instructions as a function of time or
 *		just randomly
 * @param	time	Time of the instruction being generated
 * @return	An integer from 0-2 representing the instruction.
 */
unsigned int getInstruction(unsigned long long time)
{
	
	if (time %2 == 0)
	return 0;
	
	else
	return 0; 
}

/**
 * @fn		openFile
 * @brief	Wrapper function for fopen
 *
 * @returns	Pointer to fd of new file.
 */
FILE* openFile()
{
	FILE* file = fopen("trace.txt", "w");
	if (file)
		return file;
	
	perror("Error in writeTrace: Error creating trace.txt\n");
	exit(EXIT_FAILURE);
}

/**
 * @fn		Fclose
 * @brief	Wrapper function for fclose()
 *
 * @param	stream	fd that is being closed
 */
void Fclose(FILE *stream)
{
	if (fclose(stream))
	{
		perror("Error in writeTrace: Error closing trace file\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * @fn		parseFlags
 * @brief	Parses the arguments of the thread for flags
 *
 * @detail	Scans each input to see if it is setting a flag. Checks for syntax and argument bounds. Warns the user
 * 		if invalid arguments are received or out-of-bound values used but just corrects or changes values to
 * 		defaults and continues execution
 *
 * @param	argc	number of arguments to parse
 * @param	argv	array of pointers to the beginning of each
 * 			argument string
 */
void parseFlags(int argc, char** argv)
{
	if (argc == 1)
		return; //If no flags were provided

	for (int i = 1; i < argc; i++) //For each string in argv
	{
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
		else if (!strcmp(argv[i], "-mult")) //If the -mult flag is asserted
		{
			if (isNumber(argv[i+1])) //Verify the user specified a number after -s flag
			{
				i++;
				multipleEntries = atoi(argv[i]);
			}
			else
				expectedNumber("mult", argv[i+1]); //If no number follows -s, alert user. Default to 1.
		}
		else if (!strcmp(argv[i], "-end")) //If the -end flag is set
		{
			if (isNumber(argv[i+1]))
			{
				i++;
				endtime = strtoull(argv[i], 0, 0);
			}
			else
				expectedNumber("end", argv[i+1]);
		}
		else
		{
			Printf("Invalid argument: %s\n", argv[i]);
		}
	}
	if (timestep < 1)
	{
		Printf("Timestep must be at least 1. Using 1.\n");
		timestep = 1;
	}
	if (multipleEntries < 1)
	{
		Printf("Cannot set -mult arg to < 1. Using 1\n");
		multipleEntries = 1;
	}
	if (endtime < 1)
	{
		Printf("Cannot set end time to < 1. Using 1\n");
		endtime = 1;
	}
}

/**
 * @fn		expectedNumber
 * @brief	Informs the user that they didn't enter a number an argument
 *
 * @param	arg	A NULL pointer or a null terminated string that contains the argument that should have been a number
 */
void expectedNumber(char* flag, char* arg)
{
	if (!arg)
		Printf("Missing parameter after -%s flag. Expected number.\n", flag);
	else
	{
		Printf("Invalid argument %s after -%s. Expected number.\n", arg, flag);
	}
}

/**
 * @fn		isNumber
 * @brief	Checks if a given string contains only numeric digits
 *
 * @param	test	String being examined
 *
 * @return	1 if the string only contains numeric digits, 0 otherwise
 */
uint8_t isNumber(const char* test)
{
	if (!test)
		return 0;
	
	//Check if every character in test is a digit
	if (strspn(test, "0123456789") == strlen(test))
	{
		return 1;
	}

	return 0;
}

/**
 * @fn		Fprintf
 * @brief	Wrapper function for fprintf to catch errors
 *
 * @param	stream	fd of the target output file
 * @param	format	A format string provided to printf
 * @param	...	All other variables required for the given format string
 */
void Fprintf(FILE* stream, char* format, ...)
{
	va_list args;
	va_start (args, format);
	int result = vfprintf(stream, format, args);
	va_end(args);

	if (result < 0) //If the print was not successful, alert user via stderr and exit
	{
		perror("Error in writeTrace: Error calling fprintf()\n");
		exit(EXIT_FAILURE);
	}
}


/**
 * @fn		Printf
 * @brief	Wrapper function for printf to catch errors
 *
 * @param	format	A format string provided to printf
 * @param	...	All other variables required for the given format string
 */
void Printf(char* format, ...)
{
	va_list args;
	va_start (args, format);
	int result = vprintf(format, args);
	va_end(args);

	if (result < 0) //If the print was not successful, alert user via stderr, collect garbage, and exit
	{
		perror("Error in writeTrace: Error calling printf()\n");
		exit(EXIT_FAILURE);
	}
}

