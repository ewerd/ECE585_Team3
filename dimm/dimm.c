

#include "dimm.h"

dimm_t *dimm_init(int groups, int banks, int rows)
{
	dimm_t *newDimm = Malloc(sizeof(dimm_t));
	newDimm->group = Malloc(groups*sizeof(bGroup_t));
	for (int i = 0; i < groups; i++)
	{
		group_init(banks, rows, &newDimm->group[i]);	
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
		group_deinit(&dimm->group[i]);
	}
	free(dimm->group);
	free(dimm);
}
