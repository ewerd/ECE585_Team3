

#ifndef GROUP_H_
#define GROUP_H_

#include "bank.h"
#include "../wrappers.h"

typedef struct {
	bank_t**		bank; //Array of banks
	unsigned		numBanks; //Number of banks in group
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
 * @return	Pointer to new group struct
 */
bGroup_t *group_init(unsigned banks, unsigned rows);

/**
 * @fn		group_deinit
 * @brief	Frees the memory allocated for the bank group
 *
 * @param	bankGroup	Target bankGroup to clean
 */
void group_deinit(bGroup_t *bankGroup);

#endif
