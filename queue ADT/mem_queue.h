/**
 * mem_queue.h - Header file for ECE585 queue
 *
 * Authors: Braden Harwood, Drew Seidel, Michael Weston, Stephen Short
 * Date: 11/11/2021
 *
 */
#ifndef _MEMQUEUE_H
#define _MEMQUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZEQUEUE 16
#define DEBUG 1

// STRUCTS AND GLOBALS

// Define Operation Enum
typedef enum _operation_e { READ, WRITE, IFETCH } operation_t;

// define row from file separation
typedef struct _cpu_command_s 
{
	long long			cpuCycle;
	operation_t 		command;
	long long 			address;		// 33 bits
	// Could make these smaller bit widths.
	unsigned int 		rows; 			// 15 bits
	unsigned int		upperColumns; 	//  8 bits
	unsigned int		banks; 			//  2 bits
	unsigned int		bankGroups; 	//  2 bits
	unsigned int		lowerColumns; 	//  3 bits
	unsigned int		byteSelect; 	//  3 bits
} inputCommand_t, *inputCommandPtr_t;

typedef struct queueItem_s 
{
	int index;
	struct queueItem_s	*prev;			// previous item in Queue
	struct queueItem_s	*next;			// next item in Queue
	inputCommand_t		command;		// pointer to row item struct w/ address and cpu cycle time

} queueItem_t, *queueItemPtr_t;


// STRUCTURE: queue is oriented so that 15 represents most recent element (back of queue) and 0 represents oldest element (front of queue)
typedef struct queue_s
{
	int					size;			// amount of items in queue
	long long			cpuCounter;		// which increment the program is on
	queueItemPtr_t		firstCommand;	// pointer to first item in queue
	queueItemPtr_t		lastCommand;	// pointer to last item in queue

} queue_t, *queuePtr_t;

// FXN PROTOTYPES

/**
* FUNCTION:			create_queue
*
* INFO:				Initiates a queue ADT to be populated by the memory simulator 
*					with queue items which hold all the information about the memory 
*					requests. This should only be called once at the beginning of a 
*					given simulation.
*
* PARAMS:			none
*
* RETURNS:			queuePtr_t (pointer to queue item ADT)
*/
queuePtr_t create_queue();

/**
* FUNCTION:			insert_queue_item
*
* INFO:				Inserts a provided queue item to the back of a provided queuePtr_t (queue pointer).
*					
*
* PARAMS:			queuePtr_t queue (queue to be inserted into)
*					inputCommandPtr_t item (item to be inserted)
*
* RETURNS:			queueItemPtr_T (pointer to newly inserted queue item)
*/
queueItemPtr_t insert_queue_item(queuePtr_t queue, inputCommandPtr_t command);

/**
* FUNCTION:			remove_queue_item
*
* INFO:				Removes an item from the queue (completed memory request)
*
*
* PARAMS:			int index (integer index of queue item to be removed)
*					queuePtr_t queue (queue to be removed from)
*
* RETURNS:			TBD (void for now)
*/
void remove_queue_item(int index, queuePtr_t queue);

/**
* FUNCTION:			print_queue
*
* INFO:				prints out one or all the item's information in the queue
*
*
* PARAMS:			queuePtr_t queue (queue to be accessed)
*					int index (integer index of item to be printed)
*					bool all (if true will print all items; default false)
*
* RETURNS:			TBD (void for now and just prints to console)
*/
void print_queue(queuePtr_t queue, int index, bool all);
#endif
