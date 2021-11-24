

#include "bank.h"
#include "../wrappers.h"
#include "dimm.h"

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
	if (bank == NULL)
	{
		Fprintf(stderr, "Error in bank.bank_canActivate(): NULL bank pointer passed.\n");
		return -2;
	}
	if (bank->state != PRECHARGED) //If the bank is not precharged or precharging
	{
		return -1;
	}

	//If bank isn't done precharging, return time until completed. Otherwise, return 0
	return (bank->nextActivate > currentTime) ? (int)(bank->nextActivate - currentTime) : 0;
}

int bank_activate(bank_t *bank, unsigned row, unsigned long long currentTime)
{
	if (bank == NULL)
	{
		Fprintf(stderr, "Error in bank.bank_activate(): NULL bank pointer passed.\n");
		return -2;
	}
	if (row >= bank->maxRows)
	{
		Fprintf(stderr, "Error in bank.bank_activate(): Row %u out of bounds. Last row is %u.\n", row, bank->maxRows-1);
		return -2;
	}
	if (bank->state != PRECHARGED)
	{
		Fprintf(stderr, "Error in bank.bank_activate(): Bank is not currently precharged and cannot activate.\n");
		return -1;
	}
	if (bank->nextActivate > currentTime)
	{
		Fprintf(stderr, "Error in bank.bank_activate(): Bank has not completed precharging. %llu cycles remain.\n", bank->nextActivate-currentTime);
		return -1;
	}

	// Legal command, calculate time until next possible commands. Update state and open row.
	bank->row = row;
	bank->state = ACTIVE;
	bank->nextRead = bank->nextWrite = currentTime + TRCD * SCALE_FACTOR;
	bank->nextPrecharge = currentTime + TRAS * SCALE_FACTOR;
	return TRCD * SCALE_FACTOR; //Return time till activation complete
}

int bank_canPrecharge(bank_t *bank, unsigned long long currentTime)
{
	if (bank == NULL)
	{
		Fprintf(stderr, "Error in bank.bank_canPrecharge(): NULL bank pointer passed.\n");
		return -2;
	}

	if (bank->state == PRECHARGED)
	{
		return -1;
	}
	return (bank->nextPrecharge > currentTime) ? bank->nextPrecharge - currentTime : 0;
}

int bank_precharge(bank_t *bank, unsigned long long currentTime)
{
	if (bank == NULL)
	{
		Fprintf(stderr, "Error in bank.bank_precharge(): NULL bank pointer passed.\n");
		return -2;
	}

	if (bank->state != ACTIVE)
	{
		Fprintf(stderr, "Error in bank.bank_precharge(): Bank is not activated.\n");
		return -1;
	}
	if (currentTime < bank->nextPrecharge)
	{
		Fprintf(stderr, "Error in bank.bank_precharge(): Bank is not ready to precharge. %llu cycles remain.\n", bank->nextPrecharge-currentTime);
		return -1;
	}

	bank->state = PRECHARGED;
	bank->nextActivate = currentTime + TRP * SCALE_FACTOR;
	return TRP * SCALE_FACTOR;
}

int bank_canRead(bank_t *bank, unsigned row, unsigned long long currentTime)
{
	if (bank == NULL)
	{
		Fprintf(stderr, "Error in bank.bank_canRead(): NULL bank pointer passed.\n");
		return -2;
	}
	if (row >= bank->maxRows)
	{
		Fprintf(stderr, "Error in bank.bank_canRead(): Row %u out of bounds. Last row is %u.\n", row, bank->maxRows-1);
		return -2;
	}

	if (bank->state != ACTIVE || bank->row != row)
	{
		return -1;
	}

	return (currentTime < bank->nextRead) ? (int)(bank->nextRead-currentTime) : 0;
}
