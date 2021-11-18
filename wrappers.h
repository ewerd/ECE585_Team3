/**
 * @file	wrappers.h
 * @brief	Helper wrapper functions to catch errors from system calls
 */

#ifndef WRAPPERS_H_
#define	WRAPPERS_H_

#include <stdlib.h>
#include <stdio.h>

/**
 * @fn		Printf
 * @brief	Wrapper function for printf to catch errors
 *
 * @param	format	A format string provided to printf
 * @param	...	All other variables required for the given format string
 */
void Printf(char* format, ...);

/**
 * @fn		Fprintf
 * @brief	Wrapper function for fprintf to catch errors
 *
 * @param	stream	fd of the target output file
 * @param	format	A format string provided to printf
 * @param	...	All other variables required for the given format string
 */
void Fprintf(FILE* stream, char* format, ...);

/**
 * @fn		Malloc
 * @brief	Wrapper function for malloc
 *
 * @param	size	Size of memory to be allocated
 * @return	Pointer to new memory
 */
void *Malloc(size_t size);

#endif
