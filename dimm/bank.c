

#include "bank.h"

void bank_init(unsigned rows, bank_t *newBank)
{
	newBank->maxRows = rows;
	newBank->row = -1;
	newBank->state = PRECHARGED;
	newBank->nextPrecharge = 0;
	newBank->nextActivate = 0;
	newBank->nextRead = 0;
	newBank->nextWrite = 0;
}
