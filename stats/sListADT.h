/**
 * @brief   Header file for sListADT.c
 *
 * @author  Stephen Short
 * @date    10/23/20
 */

#ifndef SLISTADT_H_
#define	SLISTADT_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct ListNode listNode_t;

struct ListNode
{
	unsigned key;
    	void *data;
    	listNode_t *next;
};

//List struct members define here.
typedef struct
{
    	listNode_t *top;
	listNode_t guardNode;
}sList_t;

/**
 * @fn		sList_create
 * @brief   	Instantiates a sList struct
 * 
 * @return  	Pointer to new sList struct
 */
sList_t *sList_create();

/**
 * @fn	    	sList_add
 * @brief   	Adds the object into the sorted list
 *
 * @param   	list	Reference to list.
 * @param	key	Value that data will be sorted by. Low to high
 * @param   	data    Data being added.
 */
void sList_add(sList_t *list, uint16_t key, void *data);

/**
 * @fn	    	sList_isEmpty
 * @brief   	Determines if the referenced list is empty.
 *
 * @param	List being analyzed.
 * @return  	True if the list is empty. 0 otherwise.
 */
bool sList_isEmpty(sList_t *list);

/**
 * @fn	    	sList_remove
 * @brief	Removes an entry from the list
 *
 * @param	index	Index of the data to be removed
 * @param   	list    List being removed from.
 * @return  	A pointer to the data removed. NULL if index is out of bounds
 */
void *sList_remove(unsigned index, sList_t *list);

/**
 * @fn	    	sList_delete
 * @brief   	Frees all dynamicly allocated memory and any remaining data in the referenced list.
 *
 * @param   	list    List to be deleted
 */
void sList_delete(sList_t *list);


#endif
