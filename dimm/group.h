

#include "bank.h"
#include "./wrappers.h"

typedef struct {
	bank_t*			bank; //Array of banks
	unsigned long long 	nextActivate; //Time available for next ACT command (tRRD_L)
	unsigned long long 	nextWrite; //Time available for next write(tCCD_L)
	unsigned long long 	nextRead; //Time available for next read(tWTR_L,tCCD_L)
}bGroup_t;

/**
 * @fn		group_init
 * @brief	Initializes a new bank group
 *
 * @param	banks	Number of banks in the group
 * @param	rows	Rows per bank
 * @param	newGroup	Target bank group
 */
void group_init(int banks, int rows, bGroup_t *newGroup);	
