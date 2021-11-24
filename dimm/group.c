

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
