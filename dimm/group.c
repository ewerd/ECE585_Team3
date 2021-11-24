

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

	if (currentTime < group->nextActivate)
	{
		Fprintf(stderr, "Error in group.group_activate(): Group not ready to activate. %llu cycles remain.\n", group->nextActivate - currentTime);
		return -1;
	}
	int activateTime = bank_activate(group->bank[bank], row, currentTime);
	if (activateTime > 0)
	{
		group->nextActivate = currentTime + TRRD_L * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in group.group_activate(): Unable to activate bank %u.\n", bank);
	}
	return activateTime;
}

int group_canPrecharge(bGroup_t *group, unsigned bank, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_canPrecharge(): Bad arguments passed.\n");
		return -2;
	}

	// There's no 'nextPrecharge' time in the group struct so just have to return time till
	// bank is ready for precharge
	return bank_canPrecharge(group->bank[bank], currentTime);
}

int group_precharge(bGroup_t *group, unsigned bank, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_precharge(): Bad arguments passed.\n");
		return -2;
	}
	
	int prechargeTime = bank_precharge(group->bank[bank], currentTime);
	if (prechargeTime < 0)
	{
		Fprintf(stderr, "Error in group.group_precharge(): Unable to precharge bank %u.\n", bank);
	}
	return prechargeTime;
}

int group_canRead(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_canRead(): Bad arguments passed.\n");
		return -2;
	}

	int bankReadTime = bank_canRead(group->bank[bank], row, currentTime);
	if (bankReadTime < 0)
	{
		return bankReadTime;
	}

	int groupReadTime = (group->nextRead > currentTime) ? group->nextRead - currentTime : 0;
	return (bankReadTime > groupReadTime) ? bankReadTime : groupReadTime;
}

int group_read(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_read(): Bad arguments passed.\n");
		return -2;
	}

	if (currentTime < group->nextRead)
	{
		Fprintf(stderr, "Error in group.group_read(): Group not ready to read. %llu cycles remain.\n", group->nextRead - currentTime);
		return -1;
	}
	int readTime = bank_read(group->bank[bank], row, currentTime);
	if (readTime > 0)
	{
		group->nextRead = currentTime + TCCD_L * SCALE_FACTOR;
		group->nextWrite = currentTime + TCCD_L * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in group.group_read(): Unable to read from bank %u.\n", bank);
	}
	return readTime;	
}

int group_canWrite(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_canWrite(): Bad arguments passed.\n");
		return -2;
	}

	int bankWriteTime = bank_canWrite(group->bank[bank], row, currentTime);
	if (bankWriteTime < 0)
	{
		return bankWriteTime;
	}

	int groupWriteTime = (group->nextWrite > currentTime) ? group->nextWrite - currentTime : 0;
	return (bankWriteTime > groupWriteTime) ? bankWriteTime : groupWriteTime;
}

int group_write(bGroup_t *group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (group_checkArgs(group, bank) < 0)
	{
		Fprintf(stderr, "Error in group.group_Write(): Bad arguments passed.\n");
		return -2;
	}

	if (currentTime < group->nextWrite)
	{
		Fprintf(stderr, "Error in group.group_Write(): Group not ready to write. %llu cycles remain.\n", group->nextWrite - currentTime);
		return -1;
	}
	int writeTime = bank_write(group->bank[bank], row, currentTime);
	if (writeTime > 0)
	{
		group->nextWrite = currentTime + TCCD_L * SCALE_FACTOR;
		group->nextRead = currentTime + (CWL + TBURST + TWTR_L) * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in group.group_write(): Unable to write to bank %u.\n", bank);
	}
	return writeTime;	
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
