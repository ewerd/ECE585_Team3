

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

int dimm_canActivate(dimm_t *dimm, unsigned bGroup, unsigned bank)
{
	return 0;
}
