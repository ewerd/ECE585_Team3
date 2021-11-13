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
	unsigned long long	cpuCycle;
	operation_t 		command;
	unsigned long long 	address;		// 33 bits
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
	unsigned long long	age;			// how long the queue item has been in the queue
	struct queueItem_s	*prev;			// previous item in Queue
	struct queueItem_s	*next;			// next item in Queue
	inputCommandPtr_t	command;		// pointer to row item struct w/ address and cpu cycle time

} queueItem_t, *queueItemPtr_t;


// STRUCTURE: queue is oriented so that 16 represents most recent element (back of queue) and 1 represents oldest element (front of queue)
typedef struct queue_s
{
	int					size;			// amount of items in queue
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
* PARAMS:			queuePtr_t queue (queue to be inserted into)
*					inputCommandPtr_t item (item to be inserted)
*
* RETURNS:			queueItemPtr_T (pointer to newly inserted queue item)
*/
queueItemPtr_t insert_queue_item(queuePtr_t queue, inputCommandPtr_t command);

/**
* FUNCTION:			peak_queue_item
*
* INFO:				Peaks at a provided queue item (queue pointer).
*					
* PARAMS:			queuePtr_t queue (queue to be inserted into)
*					queuePtr_t index (index of desired item)
*
* RETURNS:			queueItemPtr_T (pointer to peaked at queue item)
*/
queueItemPtr_t peak_queue_item(int index, queuePtr_t queue);

/**
* FUNCTION:			remove_queue_item
*
* INFO:				Removes an item from the queue (completed memory request)
*
* PARAMS:			int index (integer index of queue item to be removed)
*					queuePtr_t queue (queue to be removed from)
*
* RETURNS:			TBD (void for now)
*/
void remove_queue_item(int index, queuePtr_t queue);

/**
* FUNCTION:			age_queue
*
* INFO:				ages all the items of the queue by a given ull increment
*
* PARAMS:			queuePtr_t queue (queue to be accessed)
*					ull increment (integer index of item to be printed)
*
* RETURNS:			N/A
*/
void age_queue(queuePtr_t queue, unsigned long long increment);

/**
* FUNCTION:			print_queue
*
* INFO:				prints out one or all the item's information in the queue
*
* PARAMS:			queuePtr_t queue (queue to be accessed)
*					int index (integer index of item to be printed)
*					bool all (if true will print all items; default false)
*
* RETURNS:			TBD (void for now and just prints to console)
*/
void print_queue(queuePtr_t queue, int index, bool all);

/**
* FUNCTION:			is_empty
*
* INFO:				checks to see if the queue is currently empty
*
* PARAMS:			queuePtr_t queue (queue to be accessed)
*
* RETURNS:			boolean true if empty, false if not empty
*/
bool is_empty(queuePtr_t queue);

/**
* FUNCTION:			is_full
*
* INFO:				checks to see if the queue is currently full
*
* PARAMS:			queuePtr_t queue (queue to be accessed)
*
* RETURNS:			boolean true if full, false if not full
*/
bool is_full(queuePtr_t queue);
#endif
