

#include <stdio.h>
#include "../wrappers.h"
#include <limits.h>
#include "sListADT.h"

sList_t *sList_create()
{
    	sList_t *list;
    	list = Malloc(sizeof(sList_t));
    	list->top = &list->guardNode;
	list->guardNode.next = &list->guardNode;
	list->guardNode.key = UINT_MAX;
    	return list;
}

void sList_add(sList_t *list, uint16_t key, void *data)
{
    	listNode_t *newNode = Malloc(sizeof(listNode_t));
	newNode->key = key;
    	newNode->data = data;
	if (key < list->top->key)
	{
		newNode->next = list->top;
		list->top = newNode;
		return;
	}
	listNode_t *current;
	for (current = list->top; current->next != &list->guardNode; current = current->next)
	{
		if(newNode->key < current->next->key)
		{
			break;
		}
	}
	newNode->next = current->next;
	current->next = newNode;
}

bool sList_isEmpty(sList_t *list)
{
    return (list->top == &list->guardNode);
}

void *sList_remove(unsigned index, sList_t *list)
{
	if (sList_isEmpty(list))
		return NULL;

	listNode_t *current = list->top;
	if (index == 0)
	{
		list->top = current->next;
		void* data = current->data;
		free(current);
		return data;
	}

	listNode_t *prev = current;
	current = current->next;
	for (int i = 1; i < index; i++)
	{
		if (current == &list->guardNode)
			break;
		prev = current;
		current = current->next;
	}
	if (current == &list->guardNode)
		return NULL;
	prev->next = current->next;
	void* data = current->data;
	free(current);
	return data;
}

void sList_delete(sList_t *list)
{
	listNode_t *current = list->top;
	while (current != &list->guardNode)
	{
		listNode_t *next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
	free(list);
}
