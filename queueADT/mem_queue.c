//
//  main.c
//  ECE_585_mem_queue
//
//  Created by Drew Seidel, Braden Harwood, Michael Weston and Stephen Short on 10/26/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "mem_queue.h"
#include "./../parser/parser.h"

/*
Preliminary queue for ECE 585 project
 
Pros:

Cons:

 */
// FUNCTION DEFINITIONS:
queuePtr_t create_queue()
{
	// variables:
	queuePtr_t queue; 
	
	// attempt allocation of queue
	if (!(queue = malloc(sizeof(queue_t))))
	{
		if (DEBUG)
		{
			fprintf(stderr, "\ncreate_queue(): could not allocate space for queue...\n");
		}
		return NULL;
	}
	else
	{
		queue->firstCommand = NULL;
		queue->size = 0;

		return queue;
	}
}

queueItemPtr_t insert_queue_item(queuePtr_t queue, inputCommandPtr_t command)
{
	// check if queue is available
	if (queue->size == 16)
	{
		if (DEBUG)
		{
			fprintf(stderr, "\ninsert_queue_item(): queue is full, cannot insert item...\n");
		}
		return NULL;
	}

	// variables:
	queueItemPtr_t queueItem;

	// attempt to allocate queueItem
	if (!(queueItem = malloc(sizeof(queueItem_t))))
	{
		if (DEBUG)
		{
			fprintf(stderr, "\ninsert_queue_item(): could not allocate data for queue item...\n");
		}
		return NULL;
	}
	else
	{
		// successful allocation; insert command into new queueItem
		queueItem->command = command;
		queueItem->prev = NULL;
		queueItem->next = NULL;
		queueItem->age = 0;

		// determine if queue is empty
		if (queue->firstCommand == NULL)
		{
			// first & last command in queue
			queue->firstCommand = queueItem;
			queue->lastCommand = queueItem;

			// indexes labeled 1 to 16 -> eldest to newest (changeable if desired)
			queueItem->index = 1; 
		}
		else
		{
			// insert into back of the queue
			// last command of queue points towards new last command
			queue->lastCommand->next = queueItem;

			// new command points to previously last command of queue and indexed 
			queueItem->prev = queue->lastCommand;
			queueItem->index = queue->lastCommand->index + 1;

			// set new queue item to back of queue (bigger index), and queue points next NULL because now back of queue
			queue->lastCommand = queueItem;
			queueItem->next = NULL;
		}

		// increment queue size and return new queue item pointer
		queue->size++;
		return queueItem;
	}
}	

queueItemPtr_t peak_queue_item(int index, queuePtr_t queue)
{
	// variables:
	queueItemPtr_t temp = queue->firstCommand;

	// go through queue and age each queue item
	for (int x = 1; x < queue->size; x++)
	{
		// index found, return pointer to queueItem
		if (x == index)
		{
			return temp;
		}

		// index not found, incrememnt to next node
		temp = temp->next;
	}
	
	// If index not found, return NULL pointer
	return NULL;
}

void remove_queue_item(int index, queuePtr_t queue)
{
	//variables:
	queueItemPtr_t next, temp = queue->firstCommand;

	bool removed = false;

	for (int x = 1; x <= queue->size; x++)
	{
		// iterate through queue and free selected index
		if (temp->index == index && !removed)
		{
			// if next isn't NULL, assign
			if (temp->next != NULL)
			{
				temp->next->prev = temp->prev;
			}
			else
			{
				queue->lastCommand = temp->prev;
				temp->prev->next = NULL;
			}

			// if prev isn't NULL, assign
			if (temp->prev != NULL)
			{
				temp->prev->next = temp->next;
			}
			else
			{
				queue->firstCommand = temp->next;
				temp->next->prev = NULL;
			}

			// once prev and next properly assigned, free temp
			next = temp->next;
			free(temp);
			temp = next;
			queue->size--;
			// set flag for index removed
			removed = true;
		}
		else
		{
			// once the desired index is removed, decrement all proceeding elements
			if (removed)
			{
				temp->index--;
			}

			// if not the last element, increment
			if (temp->next != NULL)
			{
				temp = temp->next;
			}
		}
	}
}

void age_queue(queuePtr_t queue, unsigned long long increment)
{
	// variables:
	queueItemPtr_t temp = queue->firstCommand;

	// go through queue and age each queue item
	for (int x = 1; x < queue->size; x++)
	{
		temp->age += increment;

		if (temp->next != NULL)
		{
			temp = temp->next;
		}
	}
}

void print_queue(queuePtr_t queue, int index, bool all)
{
	// variables:
	queueItemPtr_t temp = queue->firstCommand;
	printf("\n\nQUEUE:\n");
	// go through queue items and print out relevant information for debugging
	for (int x = 1; x < queue->size; x++)
	{
		if (x == index || all) 
		{
			// shortener variable for readability
			inputCommandPtr_t cmd = temp->command;

			printf("Item: %d\t, Time = %10lld, Address = 0x%010llX\n", temp->index, cmd->cpuCycle, cmd->address);

			if (temp->next != NULL)
			{
				temp = temp->next;
			}
		}
	}
	printf("\n\n");
}

bool is_empty(queuePtr_t queue)
{
	return (queue->size == 0);
}

bool is_full(queuePtr_t queue)
{
	return (queue->size == 16);
}