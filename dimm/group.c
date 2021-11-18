

void group_init(int banks, int rows, bGroup_t *newGroup)
{
	newGroup->bank = Malloc(banks * sizeof(bank_t));
	for (int i = 0; i < banks; i++)
	{
		bank_init(rows, &newGroup->bank[i]);
	}
	newGroup->nextActivate = 0;
	newGroup->nextWrite = 0;
	newGroup->nextRead = 0;
}
