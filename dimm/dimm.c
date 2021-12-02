

#include "dimm.h"

/*
 * Helper function declarations
 */
int dimm_checkArgs(dimm_t *dimm, unsigned group);

dimm_t *dimm_init(unsigned groups, unsigned banks, unsigned rows)
{
	// Allocate dimm_t object and subsequently its bank groups and banks
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
	// dimm isn't allocated, return
	if (dimm == NULL)
		return;
	
	// if dimm is allocated, deallocate or free() its groups first, then free() the actual dimm
	for (unsigned i = 0; i < dimm->numGroups; i++)
	{
		group_deinit(dimm->group[i]);
	}
	// free() in this order because groups are nested under dimm
	free(dimm->group);
	free(dimm);
}

int dimm_canActivate(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	// checks for a valid dimm pointer and group number, will return error -2 if bad arguments
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
	// checks for a valid dimm pointer and group number, will return error -2 if bad arguments
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Bad arguments.\n");
		return -2;
	}
	
	if (currentTime < dimm->nextActivate)
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Dimm not ready to activate. %llu cycles remain.\n",dimm->nextActivate - currentTime);
		return -1;
	}
	
	// dimm_activate calls group_activate which then calls bank_activate to check that a particular bank can be activated
	int activateTime = group_activate(dimm->group[group], bank, row, currentTime);
	if (activateTime > 0)
	{
		dimm->nextActivate = currentTime + TRRD_S * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in dimm.dimm_activate(): Could not activate group %u.\n", group);
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

int dimm_precharge(dimm_t *dimm, unsigned group, unsigned bank, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_precharge(): Bad arguments.\n");
		return -2;	
	}
	
	int prechargeTime = group_precharge(dimm->group[group], bank, currentTime);
	if (prechargeTime < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_precharge(): Unable to precharge group %u.\n", group);
	}
	return prechargeTime;
}

int dimm_canRead(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group) < 0)
	{
		Fprintf(stderr, "Error in dimm.dimm_canRead(): Bad arguments.\n");
		return -2;	
	}

	int groupReadTime = group_canRead(dimm->group[group], bank, row, currentTime);
	if (groupReadTime < 0)
	{
		return groupReadTime;
	}
	
	int dimmReadTime = (dimm->nextRead > currentTime) ? dimm->nextRead - currentTime : 0;
	return (dimmReadTime < groupReadTime) ? groupReadTime : dimmReadTime;
}

int dimm_read(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_read(): Bad arguments.\n");
		return -2;	
	}

	
	if (currentTime < dimm->nextRead)
	{
		Fprintf(stderr, "Error in dimm.dimm_read(): Dimm not ready to read. %llu cycles remain.\n",dimm->nextRead - currentTime);
		return -1;
	}
	int readTime = group_read(dimm->group[group], bank, row, currentTime);
	if (readTime > 0)
	{
		dimm->nextRead = dimm->nextWrite = currentTime + TCCD_S * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in dimm.dimm_read(): Could not read from group %u.\n", group);
	}
	return readTime;
}

int dimm_canWrite(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_canWrite(): Bad arguments.\n");
		return -2;	
	}

	int groupWriteTime = group_canWrite(dimm->group[group], bank, row, currentTime);
	if (groupWriteTime < 0)
	{
		return groupWriteTime;
	}
	
	int dimmWriteTime = (dimm->nextWrite > currentTime) ? dimm->nextWrite - currentTime : 0;
	return (dimmWriteTime < groupWriteTime) ? groupWriteTime : dimmWriteTime;
}

int dimm_write(dimm_t *dimm, unsigned group, unsigned bank, unsigned row, unsigned long long currentTime)
{
	if (dimm_checkArgs(dimm, group < 0))
	{
		Fprintf(stderr, "Error in dimm.dimm_write(): Bad arguments.\n");
		return -2;	
	}

	if (currentTime < dimm->nextWrite)
	{
		Fprintf(stderr, "Error in dimm.dimm_write(): Dimm not ready to write. %llu cycles remain.\n",dimm->nextWrite - currentTime);
		return -1;
	}
	int writeTime = group_write(dimm->group[group], bank, row, currentTime);
	if (writeTime > 0)
	{
		dimm->nextWrite = currentTime + TCCD_S * SCALE_FACTOR;
		dimm->nextRead = currentTime + (CWL + TBURST + TWTR_S) * SCALE_FACTOR;
	}
	else
	{
		Fprintf(stderr, "Error in dimm.dimm_write(): Could not write to group %u.\n", group);
	}
	return writeTime;
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
