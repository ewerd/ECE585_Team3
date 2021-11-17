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
#include "../wrappers.h"
#include "mem_queue.h"

// FUNCTION DEFINITIONS:
queuePtr_t create_queue(unsigned maxSize)
{
	// variables:
	queuePtr_t queue; 
	
	// attempt allocation of queue
	if (!(queue = malloc(sizeof(queue_t))))
	{
		#ifdef DEBUG
		Fprintf(stderr, "\ncreate_queue(): could not allocate space for queue...\n");
		#endif
		return NULL;
	}
	else
	{
		queue->firstCommand = NULL;
		queue->size = 0;
		queue->maxSize = maxSize;
		return queue;
	}
}

queueItemPtr_t insert_queue_item(queuePtr_t queue, void *item)
{
	// check if queue is available
	if (queue->size == queue->maxSize)
	{
		Fprintf(stderr, "\nError in mem_queue.insert_queue_item(): queue is full, cannot insert item...\n");
		return NULL;
	}

	// variables:
	queueItemPtr_t queueItem;

	// attempt to allocate queueItem
	if (!(queueItem = malloc(sizeof(queueItem_t))))
	{
		Fprintf(stderr, "\nError in mem_queue.insert_queue_item(): could not allocate data for queue item...\n");
		return NULL;
	}
	else
	{
		// successful allocation; insert command into new queueItem
		queueItem->item = item;
		queueItem->prev = NULL;
		queueItem->next = NULL;
		queueItem->age = 0;

		// determine if queue is empty
		if (queue->firstCommand == NULL)
		{
			#ifdef DEBUG
				Printf("Queue: Queue empty, setting first and last command pointers to new command\n");
			#endif
			// first & last command in queue
			queue->firstCommand = queueItem;
			queue->lastCommand = queueItem;

			// indexes labeled 1 to 16 -> eldest to newest (changeable if desired)
			queueItem->index = 1; 
		}
		else
		{
			#ifdef DEBUG
				Printf("Queue: inserting into back of queue\n");
			#endif
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
		#ifdef DEBUG
			Printf("Queue: Incrementing size. New size:%d\n", queue->size);
		#endif
		return queueItem;
	}
}	

void* peak_queue_item(int index, queuePtr_t queue)
{
	#ifdef DEBUG
		Printf("Queue: Peaking at queue index %d. Size is %d\n", index, queue->size);
	#endif
	// variables:
	queueItemPtr_t temp = queue->firstCommand;

	// go through queue and age each queue item
	for (int x = 1; x <= queue->size; x++)
	{
		// index found, return pointer to queueItem
		if (x == index)
		{
			return temp->item;
		}

		// index not found, incrememnt to next node
		temp = temp->next;
	}

	#ifdef DEBUG
		Printf("Queue: Index %d is not in queue\n", index);
	#endif
	
	// If index not found, return NULL pointer
	return NULL;
}

unsigned long long getAge(unsigned index, queuePtr_t queue)
{
	if (index > queue->size)
		return ULLONG_MAX;

	queueItemPtr_t node = queue->firstCommand;
	for (int i = 1; i < index; i++)
	{
		node = node->next;
	}

	return node->age;
}

void* remove_queue_item(int index, queuePtr_t queue)
{
	//variables:
	queueItemPtr_t next, temp = queue->firstCommand;
	
	void* removedItem = NULL;

	for (int x = 1; x <= queue->size; x++)
	{
		// iterate through queue and free selected index
		if (temp->index == index && removedItem == NULL)
		{
			// if next isn't NULL, assign
			if (temp->next != NULL)
			{
				temp->next->prev = temp->prev;
			}
			else
			{
				queue->lastCommand = temp->prev;
				//temp->prev->next = NULL;
			}

			// if prev isn't NULL, assign
			if (temp->prev != NULL)
			{
				temp->prev->next = temp->next;
			}
			else
			{
				queue->firstCommand = temp->next;
				//temp->next->prev = NULL;
			}

			// once prev and next properly assigned, free temp
			next = temp->next;
			removedItem = temp->item; //Save item for return
			free(temp);
			temp = next;
		}
		else
		{
			// once the desired index is removed, decrement all proceeding elements
			if (removedItem != NULL)
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
	if (removedItem != NULL)
	{
		queue->size--;
	}
	return removedItem;
}

void age_queue(queuePtr_t queue, unsigned long long increment)
{
	// variables:
	queueItemPtr_t temp = queue->firstCommand;

	// go through queue and age each queue item
	for (int x = 1; x <= queue->size; x++)
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
	Printf("\n\nQUEUE:\n");
	// go through queue items and print out relevant information for debugging
	for (int x = 1; x <= queue->size; x++)
	{
		if (x == index || all) 
		{
			// shortener variable for readability
			Printf("Index: %2u\t, Age = %10llu, Address = 0x%llX\n", temp->index, temp->age, (unsigned)&temp->item);

			if (temp->next != NULL)
			{
				temp = temp->next;
			}
		}
	}
	Printf("\n\n");
}

bool is_empty(queuePtr_t queue)
{
	return (queue->size == 0);
}

bool is_full(queuePtr_t queue)
{
	return (queue->size == queue->maxSize);
}
