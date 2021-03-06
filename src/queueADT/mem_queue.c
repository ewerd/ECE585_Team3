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
#include "../wrappers/wrappers.h"
#include "mem_queue.h"

// Private helper functions
static queueItemPtr_t newNode(void* item, uint8_t age);

// FUNCTION DEFINITIONS:
queuePtr_t create_queue(unsigned maxSize)
{
	// variables:
	queuePtr_t queue; 
	
	// attempt allocation of queue
	queue = Malloc(sizeof(queue_t));
	queue->firstCommand = NULL;
	queue->size = 0;
	queue->maxSize = maxSize;
	return queue;
}

void clean_queue(queuePtr_t list)
{
	if (list == NULL)
		return;

	while (!is_empty(list))
	{
		free(remove_queue_item(1,list));
	}
	free(list);
}

/**
 * @fn		newNode
 * @brief	Helper function to allocate and initialize a new list node
 *
 * @param	item	Pointer to the item being stored in the node
 * @param	age	Age of the item in the node
 */
static queueItemPtr_t newNode(void* item, uint8_t age)
{
	queueItemPtr_t queueItem = Malloc(sizeof(queueItem_t));
	// successful allocation; insert command into new queueItem
	queueItem->item = item;
	queueItem->prev = NULL;
	queueItem->next = NULL;
	queueItem->age = age;
	queueItem->timeInQueue = 0;
	return queueItem;
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
	queueItemPtr_t queueItem = newNode(item, 0);

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

void *sorted_insert_queue(void *item, uint8_t age, queuePtr_t queue)
{
	#ifdef DEBUG
		Printf("Queue:Inserting item with age %u into list with size %u\n",(unsigned)age,queue->size);
	#endif
	//If queue is full, don't insert and return NULL
	if (queue->size == queue->maxSize)
	{
		#ifdef DEBUG
			Printf("Queue:List is full with %u items. Item will not be inserted.\n", queue->maxSize);
		#endif
		return NULL;
	}

	//Create new queue item
	queueItemPtr_t newItem = newNode(item, age);

	#ifdef DEBUG
		Printf("Queue:New size after insertion will be %u\n",queue->size+1);
	#endif
	queue->size++;
	//If queue empty:
	if (queue->firstCommand == NULL)
	{
		#ifdef DEBUG
			Printf("Queue:List is empty. Inserting item\n");
		#endif
		queue->firstCommand = queue->lastCommand = newItem;
		newItem->index = 1;
		return item;
	}
	
	bool inserted = false;

	for (queueItemPtr_t current = queue->firstCommand; current != NULL; current = current->next)
	{
		#ifdef DEBUG
			Printf("Queue:Comparing to item with age %u\n", current->age);
		#endif
		if (newItem->age < current->age && !inserted)
		{
			#ifdef DEBUG
				Printf("Queue:Inserting item with age %u here at index %u.\n",newItem->age,current->index);
			#endif
			if (current->prev != NULL)
			{
				current->prev->next = newItem;
			}
			else
			{
				queue->firstCommand = newItem;
			}
			newItem->prev = current->prev;
			newItem->next = current;
			newItem->index = current->index;
			current->prev = newItem;
			inserted = true;
		}

		if (inserted)
		{
			#ifdef DEBUG
				Printf("Queue:Moving item from index %u to %u.\n", current->index, current->index+1);
			#endif
			current->index++;
		}
	}

	if (inserted)
	{
		return item;
	}

	#ifdef DEBUG
		Printf("Queue:New age not < any age in queue. Inserting in back at index %u.\n",queue->lastCommand->index+1);
	#endif

	queue->lastCommand->next = newItem;
	newItem->prev = queue->lastCommand;
	newItem->index = queue->lastCommand->index+1;
	queue->lastCommand = newItem;
	return item;
}

void* peak_queue_item(unsigned index, queuePtr_t queue)
{
	#ifdef DEBUG
		Printf("Queue: Peaking at queue index %u. Size is %d\n", index, queue->size);
	#endif
	if (index == 0)
	{
		Fprintf(stderr, "Warning in queue.peak_queue_item(): Passed index as 0. This data structure is base 1. SHAME!\n");
		index = 1;
	}
	
	// variables:
	queueItemPtr_t temp = queue->firstCommand;

	// go through queue and age each queue item
	for (unsigned x = 1; x <= queue->size; x++)
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
		Printf("Queue: Index %u is not in queue\n", index);
	#endif
	
	// If index not found, return NULL pointer
	return NULL;
}

uint8_t getAge(unsigned index, queuePtr_t queue)
{
	if (index == 0)
	{
		Fprintf(stderr, "Warning in queue.getAge(): Passed 0 as index to a data structure that start at index 1. SHAME!\n");
		index = 1;
	}
	if (index > queue->size)
		return 0;

	queueItemPtr_t node = queue->firstCommand;
	for (int i = 1; i < index; i++)
	{
		node = node->next;
	}

	return node->age;
}

int setAge(unsigned index, uint8_t age, queue_t *queue)
{
	if (index == 0)
	{
		Fprintf(stderr, "Warning in queue.setAge(): Passed 0 as index to a data structure that start at index 1. SHAME!\n");
		index = 1;
	}

	queueItem_t *entry = queue->firstCommand;
	for (int i = 1; i < index; i++)
	{
		if (entry == NULL)
			return -1;

		entry = entry->next;
	}

	entry->age = age;
	return 0;
}

uint16_t getTimeInQueue(unsigned index, queue_t *queue)
{
	if (index == 0)
	{
		Fprintf(stderr, "Warning in queue.getTimeInQueue(): Passed 0 as index to a data structure that start at index 1. SHAME!\n");
		index = 1;
	}
	if (index > queue->size)
	{
		Fprintf(stderr, "Warning in queue.getTimeInQueue(): Passed out of bounds index.\n");
		return 0;
	}

	queueItem_t *entry = queue->firstCommand;
	for (unsigned i = 1; i<index; i++)
	{
		entry = entry->next;
	}
	return entry->timeInQueue;
}

void* remove_queue_item(int index, queuePtr_t queue)
{
	if (index == 0)
	{
		Fprintf(stderr, "Warning in queue.remove_queue_item(): Passed 0 as index to a data structure that start at index 1. SHAME!\n");
		index = 1;
	}

	//variables:
	queueItemPtr_t next, temp = queue->firstCommand;
	
	void* removedItem = NULL;

	for (int x = 1; x <= queue->size; x++)
	{
		// iterate through queue and free selected index
		if (temp->index == index && removedItem == NULL)
		{
			#ifdef DEBUG
			Printf("Queue.remove_queue_item(): Removing item at index %u.\n", index);
			#endif
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

void age_queue(queuePtr_t queue, uint8_t increment)
{
	// go through queue and age each queue item
	for (queueItemPtr_t current = queue->firstCommand; current != NULL; current = current->next)
	{
		#ifdef DEBUG
		Printf("queue.age_queue(): Decrementing age: %u by %u\n", current->age, increment);
		#endif
		if (current->age < increment)
		{
			current->age = 0;
		}
		else
		{
			current->age -= increment;
		}
		#ifdef DEBUG
		Printf("queue.age_queue(): New age: %u\n", current->age);
		#endif

		current->timeInQueue = ((uint16_t)(current->timeInQueue + increment) < current->timeInQueue) ? USHRT_MAX : current->timeInQueue + increment;
	}
}

void print_queue(queuePtr_t queue, int index, bool all)
{
	if (index == 0)
	{
		Fprintf(stderr, "Warning in queue.print_queue(): Passed 0 as index to a data structure that start at index 1. SHAME!\n");
		index = 1;
	}
	// variables:
	queueItemPtr_t temp = queue->firstCommand;
	Printf("\nQUEUE(Size %u):\n",queue->size);
	// go through queue items and print out relevant information for debugging
	for (int x = 1; x <= queue->size; x++)
	{
		if (x == index || all) 
		{
			// shortener variable for readability
			Printf("Index: %2u\t, Age:%3u, TIQ:%u, Address:0x%llX\n", temp->index, temp->age, temp->timeInQueue, (unsigned)&temp->item);

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
