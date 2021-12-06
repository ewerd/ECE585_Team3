#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Quick Script To Show Statistic Difference
    Project ECE 585, Demonstrated on Dec 6, 2021
    Author: Drew Seidel (Dseidel@pdx.edu)
*/

FILE *input;
bool print_stat = false;

int main(int argc, char *argv[])
{

    char str[100]; // string for each row in file

    if (argc == 1)
    {
        input = stdin;
    }
    else
    {
        fprintf(stderr, "should only be use for piping. something went awry.");
    }

    while ((fgets(str, 128, input) != NULL))
    {

        if ((str[0] == '-') && !print_stat)
        {
            print_stat = true;
        }

        if (print_stat)
        {
            printf("%s", str);
        }
    }
}
