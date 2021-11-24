

#include "dimm.h"

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
	//Error checking that bGroup is in range
	if (group >= dimm->numGroups)
	{
		Fprintf(stderr, "Error in dimm.dimm_canActivate(): Group index %u out of bounds.Last group is %u.\n",group,dimm->numGroups-1);
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
