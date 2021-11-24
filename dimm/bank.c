

#include "bank.h"
#include "../wrappers.h"

bank_t *bank_init(unsigned rows)
{
	bank_t* newBank = Malloc(sizeof(bank_t));
	newBank->maxRows = rows;
	newBank->row = 0;
	newBank->state = PRECHARGED;
	newBank->nextPrecharge = 0;
	newBank->nextActivate = 0;
	newBank->nextRead = 0;
	newBank->nextWrite = 0;
	return newBank;
}

void bank_deinit(bank_t *bank)
{
	free(bank);
}

int bank_canActivate(bank_t *bank, unsigned long long currentTime)
{
	if (bank->state != PRECHARGED) //If the bank is not precharged or precharging
	{
		return -1;
	}

	//If bank isn't done precharging, return time until completed. Otherwise, return 0
	return (bank->nextActivate > currentTime) ? (int)(bank->nextActivate - currentTime) : 0;
}
