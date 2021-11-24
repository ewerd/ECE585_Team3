

#include "group.h"

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
	//Error check bank number
	if (bank >= group->numBanks)
	{
		Fprintf(stderr, "Error in group.group_canActivate(): Bank %u out of bounds. Last bank is %u.\n", bank, group->numBanks-1);
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
