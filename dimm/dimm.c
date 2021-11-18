



dimm_t *dimm_init(int groups, int banks, int rows)
{
	dimm_t *newDimm = Malloc(sizeof(dimm_t));
	newDimm->group = Malloc(groups*sizeof(bGroup_t));
	for (int i = 0; i < groups)
	{
		group_init(banks, rows, &newDimm->group[i]);	
	}
	newDimm->nextWrite = 0;
	newDimm->nextRead = 0;
	newDimm->nextActivate = 0;
	return newDimm;
}
