

#include "group.h"
#include "dimm.h"

/*
 * Helper function declarations
 */
int group_checkArgs(bGroup_t *group, unsigned bank);

bGroup_t *group_init(unsigned banks, unsigned rows)
{
	bGroup_t* newGroup = Malloc(sizeof(bGroup_t));
	newGroup->bank = Malloc(banks * sizeof(bank_t*));
	for (int i = 0; i < banks; i++)
	{
		newGroup->bank[i]=bank_init(rows);
	}
	newGroup->numBanks = banks;
	newGroup->nextActivate = 0;
	newGroup->nextWrite = 0;
	newGroup->nextRead = 0;
	return newGroup;
}

void group_deinit(bGroup_t *bankGroup)
{
	for (unsigned i = 0; i < bankGroup->numBanks; i++)
	{
		bank_deinit(bankGroup->bank[i]);
	}
	free(bankGroup->bank);
	free(bankGroup);
}

int group_canActivate(bGroup_t *group, unsigned bank, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_canActivate(): Bad arguments passed.\n");
		return -2;
	}

	int bankActivateTime = bank_canActivate(group->bank[bank], currentTime);
	if (bankActivateTime < 0) //If bank cannot be activated, return error code
	{
		return bankActivateTime;
	}
	
	int groupActivateTime = (group->nextActivate > currentTime) ? group->nextActivate - currentTime : 0;
	return (groupActivateTime > bankActivateTime) ? groupActivateTime : bankActivateTime;
}

int group_activate(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_activate(): Bad arguments passed.\n");
		return -2;
	}

	int activateTime = bank_activate(group->bank[bank], row, currentTime);
	if (activateTime > 0)
	{
		group->nextActivate = currentTime + TRRD_L * SCALE_FACTOR;
	}
	return activateTime;
}

/**
 * @fn		group_checkArgs
 * @brief	Helper function to check if pointer is NULL and bank is within bounds
 *
 * @param	group	Pointer to a group
 * @param	bank	Bank number in that group
 * @return	0 if the arguments are valid, -1 otherwise
 */
int group_checkArgs(bGroup_t *group, unsigned bank)
{
	if (group == NULL)
	{
		Fprintf(stderr, "Error in group.group_checkArgs(): Group pointer is NULL.\n");
		return -1;
	}

	//Error check bank number
	if (bank >= group->numBanks)
	{
		Fprintf(stderr, "Error in group.group_checkArgs(): Bank %u out of bounds. Last bank is %u.\n", bank, group->numBanks-1);
		return -1;
	}

	return 0;
}
