/**
 * preliminary_parser.h - Header file for ECE585 parser
 *
 * Authors: Braden Harwood, Drew Seidel, Michael Weston, Stephen Short
 * Date: 10/29/2021
 *
 */
#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXTIME			100
#define MAXADDRESS		128

// STRUCTS AND GLOBALS
struct row_info {
	int					command;
	int					index;
	char				time[MAXTIME];
	char				address[MAXADDRESS];
	struct row_info		*prev;
	struct row_info		*next;
};

// FXN PROTOTYPES
void readfile(int *read_time, int *command_type, long long *address, int print);
#endif
