

#include "stats.h"
#include <stdlib.h>
#include "../wrappers.h"

uint8_t init_Stats()
{
	totalRequests = 0;
	fchRequests = 0;
	rdRequests = 0;
	wrRequests = 0;
	data = sList_create();
	return 0;
}

void clean_Stats()
{
	sList_delete(data);
}

void addRequest(request_t *request)
{
	sList_add(data, request->timeInQueue, (void*)request);
	totalRequests++;
	switch (request->type)
	{
		case IFETCH:
			fchRequests++;
			break;
		case RD:
			rdRequests++;
			break;
		case WR:
			wrRequests++;
			break;
	}
}

void displayStats(FILE* output)
{
	if (sList_isEmpty(data))
	{
		Fprintf(output, "No statistics to calculate.\n");
		return;
	}

	uint16_t totMin, totMax, fchMin, fchMax, rdMin, rdMax, wrMin, wrMax;
	totMin = fchMin = rdMin = wrMin = UINT16_MAX;
	totMax = fchMax = rdMax = wrMax = 0;
	float totAvg, fchAvg, rdAvg, wrAvg;
	totAvg = fchAvg = rdAvg = wrAvg = 0;
	bool totOdd, fchOdd, rdOdd, wrOdd;
	totOdd = (totalRequests % 2) ? true : false;
	fchOdd = (fchRequests % 2) ? true : false;
	rdOdd = (rdRequests % 2) ? true : false;
	wrOdd = (wrRequests % 2) ? true : false;
	uint16_t totMed1, totMed2, fchMed1, fchMed2, rdMed1, rdMed2, wrMed1, wrMed2;
	totMed1 = totMed2 = fchMed1 = fchMed2 = rdMed1 = rdMed2 = wrMed1 = wrMed2 = 0;
	unsigned long totTillMed, fchTillMed, rdTillMed, wrTillMed;
	totTillMed = totalRequests/2;
	fchTillMed = fchRequests/2;
	rdTillMed = rdRequests/2;
	wrTillMed = wrRequests/2;
	for (request_t *request = sList_remove(0, data); request != NULL; request = sList_remove(0, data))
	{
		switch (request->type)
		{
			case IFETCH:
			if (request->timeInQueue < fchMin)
				fchMin = request->timeInQueue;
			if (request->timeInQueue > fchMax)
				fchMax = request->timeInQueue;
			fchAvg += ((float)request->timeInQueue)/((float)fchRequests);
			if (fchMed1 == 0)
			{
				if (fchOdd && fchTillMed == 0)
				{
					fchMed1 = request->timeInQueue;
				}
				else if (!fchOdd && fchTillMed == 1)
				{
					fchMed2 = request->timeInQueue;
				}
				else if (!fchOdd && fchTillMed == 0)
				{
					fchMed1 = request->timeInQueue;
				}
				fchTillMed--;
			}

			case RD:
			if (request->timeInQueue < rdMin)
				rdMin = request->timeInQueue;
			if (request->timeInQueue > rdMax)
				rdMax = request->timeInQueue;
			rdAvg += ((float)request->timeInQueue)/((float)fchRequests);
			if (rdMed1 == 0)
			{
				if (rdOdd && rdTillMed == 0)
				{
					rdMed1 = request->timeInQueue;
				}
				else if (!rdOdd && rdTillMed == 1)
				{
					rdMed2 = request->timeInQueue;
				}
				else if (!rdOdd && rdTillMed == 0)
				{
					rdMed1 = request->timeInQueue;
				}
				rdTillMed--;
			}

			case WR:
			if (request->timeInQueue < wrMin)
				wrMin = request->timeInQueue;
			if (request->timeInQueue > wrMax)
				wrMax = request->timeInQueue;
			wrAvg += ((float)request->timeInQueue)/((float)wrRequests);
			if (wrMed1 == 0)
			{
				if (wrOdd && wrTillMed == 0)
				{
					wrMed1 = request->timeInQueue;
				}
				else if (!wrOdd && wrTillMed == 1)
				{
					wrMed2 = request->timeInQueue;
				}
				else if (!wrOdd && wrTillMed == 0)
				{
					wrMed1 = request->timeInQueue;
				}
				wrTillMed--;
			}

		}
		if (request->timeInQueue < totMin)
			totMin = request->timeInQueue;
		if (request->timeInQueue > totMax)
			totMax = request->timeInQueue;
		totAvg += ((float)request->timeInQueue)/((float)totalRequests);
		if (totMed1 == 0)
		{
			if (totOdd && totTillMed == 0)
			{
				totMed1 = request->timeInQueue;
			}
			else if (!totOdd && totTillMed == 1)
			{
				totMed2 = request->timeInQueue;
			}
			else if (!totOdd && totTillMed == 0)
			{
				totMed1 = request->timeInQueue;
			}
			totTillMed--;
		}
		free(request);
	}
	Fprintf(output, "--------------------STATISTICS--------------------\n");
	if (fchRequests)
	{
		Fprintf(output, "--IFETCHES:\n");
		Fprintf(output, "Min:%u\n", fchMin);
		Fprintf(output, "Max:%u\n", fchMax);
		Fprintf(output, "Average:%.3f\n", fchAvg);
		Fprintf(output, "Median:%.1f\n", (fchOdd) ? fchMed1 : ((float)(fchMed1+fchMed2))/(float)2);
		Fprintf(output, "-------------------------------------------------\n");
	}
	if (rdRequests)
	{
		Fprintf(output, "--READS:\n");
		Fprintf(output, "Min:%u\n", rdMin);
		Fprintf(output, "Max:%u\n", rdMax);
		Fprintf(output, "Average:%.3f\n", rdAvg);
		Fprintf(output, "Median:%.1f\n", (rdOdd) ? rdMed1 : ((float)(rdMed1+rdMed2))/(float)2);
		Fprintf(output, "-------------------------------------------------\n");
	}
	if (wrRequests)
	{
		Fprintf(output, "--WRITES:\n");
		Fprintf(output, "Min:%u\n", wrMin);
		Fprintf(output, "Max:%u\n", wrMax);
		Fprintf(output, "Average:%.3f\n", wrAvg);
		Fprintf(output, "Median:%.1f\n", (wrOdd) ? wrMed1 : ((float)(wrMed1+wrMed2))/(float)2);
		Fprintf(output, "-------------------------------------------------\n");
	}
	Fprintf(output, "----------------------TOTALS----------------------\n");
	Fprintf(output, "Min:%u\n", totMin);
	Fprintf(output, "Max:%u\n", totMax);
	Fprintf(output, "Average:%.3f\n", totAvg);
	Fprintf(output, "Median:%.1f\n", (totOdd) ? totMed1 : ((float)(totMed1+totMed2))/(float)2);
	Fprintf(output, "-------------------------------------------------\n");
}
