//
//  main.c
//  ECE_585_Parse
//
//  Created by Drew Seidel, Braden Hardwood, Michael Weston and Stephen Short on 10/26/21.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define SIZEARRAYS 10

void readfile(int *read_time, int *command_type, long *address, int print);
int concat(int a, int b, int c);

/*
Preliminary parser for file type of ECE 585 project
 
Pros:
-readfile function parses time into int
-readfile function deciphers instruction command
-readfile function converts hex string to hex value
-does what it needs to for Friday's 10/29 checkpoint
-^could be desirable and could not be desirable
 
 Cons:
 -I ask about printing before the filename which makes no sense
 -Memory allocation = basic and poor
 -No structures yet
 -Works only for time < 100 currently, easy to fix and add to
 -One long function for readfile, and a short one for concanate
 -Make seperate functions for data conversions in readfile perhaps
 -Not greatly written
 -Data is lost each iteration through the file (not stored in array etc.)
 -^could be desirable and could not be desirable

 
 */


int main()
{

    //make below data into structure
    int rd_time = 0; //read time from file
    int *read_time = &rd_time;

    int command = 0; //FETCH, READ, or WRITE. Int for now.
    int *command_type = &command;

    long addrs = 0x00000000; //Memory address being written, read, or fetched to/from
    long *address = &addrs;

    char userInput[2];
    int print = 0; //print option. Change this to bool

    printf("Welcome to the preliminary parser!\n");
    printf("Press P to print parsed values\nPress another key to merely read file");
    printf("\nEnter here: ");

    //fgets(userInput,2,stdin);   //User inputs option.
    scanf("%s", &userInput[0]);

    //does the user want to print, or just obtain data
    if ((!strncmp(userInput, "p", 1)) || (!strncmp(userInput, "P", 1)))
    {
        print = 1;
    }
    else
    {
        print = 0;
    }
    
    //read file function
    readfile(read_time, command_type, address, print);

    return 0;
}

void readfile(int *read_time, int *command_type, long *address, int print)
{

    char file_name[SIZEARRAYS] = ""; //will read in the name of the file.
    FILE *fp;                        //file pointer
    char str[100];                   //string for each row in file
    char hex_string[100];            //hexidecimal string to be converted to long
    char request[SIZEARRAYS];        //string to print type of request
    
    //time digits one through three to be converted to integer
    //current text file uses just two digits for time so far
    int first_digit = 0;
    int second_digit = 0;
    int third_digit = 0;
    

    //user input filename
    printf("Enter File Name: ");
    scanf("%s", file_name);

    //open file
    fp = fopen(file_name, "r");

    if (fp == NULL)
    { //file open operation check

        printf("\nFile open not successful.\n\n");
        fclose(fp);
        fp = NULL;
        return;
    }

    
    else
    {

        printf("\n\n");
        if (print == 1)
        {
            printf("Requests are as follows:\n\n");
        }

        //while not the enf of file
        while ((fgets(str, 100, fp) != NULL))
        {
            
            //to be expanded upon for when times get bigger than 100
            //do we create our own test files?
            //easy to recreate the following if commands for this filetype
            if (str[2] != ' ')
            {
                printf("error for now. Fix to work with greater files: ");
                
                //first_digit =0;      //three digits at least (assume 3 at most for now)
                //fix later
            }
            
            
            //works for current formatting of file, two integer time numbers
            if (str[2] == ' ')
            {
                //PUT ALL THIS BELOW IN ANOTHER FUNCTION
                //ONE FUNCTION FOR TWO DIGIT TIME AND THREE DIGIT
                //WILL INCREASE READABILITY AND SENSE
                // 2 digits file as is now
                first_digit = 0; //first digit does not exist
                second_digit = str[0] - '0';    //convert to int
                third_digit = str[1] - '0';     //convert to int
                *read_time = concat(first_digit, second_digit, third_digit); //concatanate

                *command_type = str[3] - '0'; //convert command type to int

                
                //convert hex string portion to long hex value
                if (str[6] == 'x' || str[6] == 'X')
                {
                    strcpy(hex_string, &str[7]);
                    sscanf(hex_string, "%lx", address);
                    //}
                }

                
                //CHANGE THIS, THIS IS DUMB
                if (*command_type == 0)
                {
                    strcpy(request, "READ ");
                }
                if (*command_type == 1)
                {
                    strcpy(request, "WRITE");
                }
                if (*command_type == 2)
                {
                    strcpy(request, "FETCH");
                }

                
                if (print == 1)
                {
                    printf("Time = %d, Command attempt = %s, Memory Address = 0x%lx\n", *read_time, request, *address);
                }
            }
        }
    }
    printf("\n");
    fclose(fp);
    fp = NULL;
}

// Function to concatenate
// three integers into one
int concat(int a, int b, int c)
{

    char s1[SIZEARRAYS];
    char s2[SIZEARRAYS];
    char s3[SIZEARRAYS];

    // Convert all three of the integers to string
    sprintf(s1, "%d", a);
    sprintf(s2, "%d", b);
    sprintf(s3, "%d", c);

    // Concatenate both strings
    strcat(s1, s2);
    strcat(s1, s3);

    // Convert the concatenated string
    // to integer
    int d = atoi(s1);

    // return the formed integer
    return d;
}

