

#include "dimm.h"

/*
 * Helper function declarations
 */
int dimm_checkArgs(dimm_t *dimm, unsigned group);

dimm_t *dimm_init(unsigned groups, unsigned banks, unsigned rows)
{
	dimm_t *newDimm = Malloc(sizeof(dimm_t));
	newDimm->group = Malloc(groups*sizeof(bGroup_t*));
	for (int i = 0; i < groups; i++)
	{
		newDimm->group[i]=group_init(banks, rows);	
	}
	newDimm->numGroups = groups;
	newDimm->nextWrite = 0;
	newDimm->nextRead = 0;
	newDimm->nextActivate = 0;
	return newDimm;
}

void dimm_deinit(dimm_t *dimm)
{
	for (unsigned i = 0; i < dimm->numGroups; i++)
	{
		group_deinit(dimm->group[i]);
	}
	free(dimm->group);
	free(dimm);
}

int dimm_canActivate(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_canActivate(): Bad arguments.\n");
		return -2;
	}

	int groupActivateTime = group_canActivate(dimm->group[group],bank,currentTime); //Get time till group ready for ACT
	if (groupActivateTime < 0) //If group cannot ACT, return error code
	{
		return groupActivateTime;
	}

	int dimmActivateTime = (dimm->nextActivate > currentTime) ? dimm->nextActivate - currentTime : 0;
	//Return the larger of the two wait times, dimmActivateTime or groupActivate time
	return (dimmActivateTime > groupActivateTime) ? dimmActivateTime : groupActivateTime;
}

int dimm_activate(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Bad arguments.\n");
		return -2;
	}

	int activateTime = group_activate(dimm->group[group], bank, row, currentTime);
	if (activateTime > 0)
	{
		dimm->nextActivate = currentTime + TRRD_S * SCALE_FACTOR;
	}
	return activateTime;
}

int dimm_canPrecharge(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_canPrecharge(): Bad arguments.\n");
		return -2;
	}

	return group_canPrecharge(dimm->group[group], bank, currentTime);
}

// ------------------------------------------------------Helper Functions-------------------------------------------------------------

/**
 * @fn		dimm_checkArgs
 * @brief	Checks for NULL pointers and out of bound group numbers
 *
 * @param	dimm	Pointer to a dimm struct
 * @param	group	Group number in that dimm struct
 * @return	-1 if an argument is bad. 0 otherwise.
 */
int dimm_checkArgs(dimm_t *dimm, unsigned group)
{
	if (dimm == NULL)
	{
		Fprintf(stderr, "Error in dimm.dimm_checkArgs(): NULL pointer passed.\n");
		return -1;
	}

	//Error checking that bGroup is in range
	if (group >= dimm->numGroups)
	{
		Fprintf(stderr, "Error in dimm.dimm_checkArgs(): Group index %u out of bounds.Last group is %u.\n",group,dimm->numGroups-1);
		return -1;
	}
	return 0;
}
