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
#include <limits.h>

// STRUCTS AND GLOBALS

typedef struct queueItem_s 
{
	unsigned index;
	unsigned long long	age;			// how long the queue item has been in the queue
	struct queueItem_s	*prev;			// previous item in Queue
	struct queueItem_s	*next;			// next item in Queue
	void			*item;			// pointer to item data

} queueItem_t, *queueItemPtr_t;


// STRUCTURE: queue is oriented so that 16 represents most recent element (back of queue) and 1 represents oldest element (front of queue)
typedef struct queue_s
{
	unsigned		size;		// amount of items in queue
	unsigned		maxSize;	// Maximum size of queue
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
* PARAMS:			maxSize	Maximum size of the queue
*
* RETURNS:			queuePtr_t (pointer to queue item ADT)
*/
queuePtr_t create_queue(unsigned maxSize);

/**
 * @fn		clean_queue
 * @brief	Frees all memory used by the list
 *
 * @param	list	Pointer to a queue_t struct
 */
void clean_queue(queuePtr_t list);

/**
* FUNCTION:			insert_queue_item
*
* INFO:				Inserts a provided queue item to the back of a provided queuePtr_t (queue pointer).
*					
* PARAMS:			queuePtr_t queue (queue to be inserted into)
*				item	A void pointer to the data to be added to the queue
*
* RETURNS:			queueItemPtr_T (pointer to newly inserted queue item)
*/
queueItemPtr_t insert_queue_item(queuePtr_t queue, void* item);

/**
 * @fn		sorted_insert_queue
 * @brief	Inserts a new item into the list sorted on age
 *
 * @param	item	Pointer to the new item
 * @param	age	Age of the new item
 * @param	queue	Pointer to the target queue
 * @return	Pointer to the new item after insertion. NULL otherwise.
 */
void *sorted_insert_queue(void *item, unsigned long long age, queuePtr_t queue);

/**
* FUNCTION:			peak_queue_item
*
* INFO:				Peaks at a provided queue item (queue pointer).
*					
* PARAMS:			queuePtr_t queue (queue to be examined)
*				index	(index of desired item)
*
* RETURNS:			Pointer to peaked at item
*/
void *peak_queue_item(unsigned index, queuePtr_t queue);

/**
 * @fn		getAge
 * @brief	Get the age of the item at a given index
 *
 * @param	queue	Target queue
 * @param	index	Index of node in queue
 * @returns	The age of the item in the node. ULLONG_MAX if the index is out of range
 */
unsigned long long getAge(unsigned index, queuePtr_t queue);

/**
 * @fn		setAge
 * @brief	Sets the age of a specific item in the queue
 *
 * @param	index	Index of the target item
 * @param	age	New age for the item
 * @param	queue	Target queue
 * @returns	0 if successfull. -1 otherwise
 */
int setAge(unsigned index, unsigned long long age, queue_t *queue);

/**
* FUNCTION:			remove_queue_item
*
* INFO:				Removes an item from the queue (completed memory request)
*
* PARAMS:			int index (integer index of queue item to be removed)
*					queuePtr_t queue (queue to be removed from)
*
* RETURNS:			Pointer to the item removed
*/
void* remove_queue_item(int index, queuePtr_t queue);

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
