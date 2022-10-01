/*
 *     restoration.c
 *     By Gillian Feder (gfeder01) and Abby Fedor (afedor02), September 2022
 *     filesofpix
 *
 *     This file uncorrupts a corrupted plain pgm file and converts it to a 
 *     raw pgm file. 
 */ 

#include <stdio.h>
#include <readaline.h>
#include <stdlib.h>
#include "atom.h"
#include "table.h"
#include <list.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>


char* splitString(char **datapp, int lengthOfString, 
                Table_T myTable, List_T *ogDigList, int *rowLength);
char* table (char *dig, char *nondig, Table_T myTable, 
                List_T *ogDigList, int nonDigCount); 
int* listToIntList(List_T *ogDigList);
void stringToIntList(char *currString, List_T *currMiniListcurrMiniList);
static void vfree(const void *key, void **value, void *cl);
void commandLoop(FILE *fp);

int main(int argc, char *argv[])
{
        assert(argc <= 2);    
        if (argc == 1) {
                assert(argv[0] != NULL);
                commandLoop(stdin);
        }
        else if (argc == 2) {
                FILE *fp = fopen(argv[1], "r");
                assert(fp != NULL);
                commandLoop(fp);
                fclose(fp);
        }
        return EXIT_SUCCESS;
}
void commandLoop (FILE *fp)
{
        /* initialization */
        char *datapp = NULL;
        // int numLines = 0;
        Table_T myTable = Table_new(100, NULL, NULL); 
        List_T ogDigList = List_list(NULL);
        char *masterSeq = NULL;
        char *tempSeq = NULL;
        int lengthOfRow, rowLength = 0;

        /* main command loop, processes every line from file */
        while (!feof(fp)) {
                // numLines++;
                int lengthOfString = readaline(fp, &datapp);
                if (lengthOfString == 0) {
                        continue;
                }
                tempSeq = splitString(&datapp, lengthOfString, 
                        myTable, &ogDigList, &rowLength);

                if (tempSeq != NULL) { /*we found injection seq */
                        free(masterSeq); 
                        masterSeq = tempSeq;
                        lengthOfRow = rowLength;    
                }

                free(datapp);
        }
        
        /*retreive last table value (digit string), push to list */
        char *finalVal = Table_get(myTable, Atom_new(masterSeq, lengthOfRow));
        ogDigList = List_push(ogDigList, finalVal);

        /* convert list of dig strings to individual lists of ints*/
        free(masterSeq);
        listToIntList(&ogDigList);

        Table_map(myTable, vfree, NULL);
        List_free(&ogDigList);
        Table_free(&myTable);
}

/* purpose: take line, split into digs and non digs strings
* params: pointer contianing line, length of line, number of lines in file, 
*         table to hash into, list to store table vals, length of row
* returns: the master injection sequence (found in table)
*/
char *splitString(char **datapp, int lengthOfString, Table_T myTable,
                 List_T *ogDigList, int *rowLength)
{
        /* initialization */
        char *fullLine;
        fullLine = *datapp;
        char *nondig;
        nondig = (char*)malloc(lengthOfString);
        assert(nondig != NULL);
        char *dig;
        dig = (char*)malloc(lengthOfString + 1); 
        assert(dig != NULL);
        int nonDigCount = 0;

        /* loop through the line, splitting digits from non digits into 
        separate strings */
        for (int i = 0; i < lengthOfString; i++) {
                if ((int) fullLine[i] >= 48 && (int)fullLine[i] <= 57) {
                        dig[i] = fullLine[i];
                }
                /* observe white space */
                else {
                        dig[i] = ' ';
                        nondig[nonDigCount] = fullLine[i];
                        nonDigCount++;
                }
        }
        dig[lengthOfString] = '\0';
        *rowLength = nonDigCount - 1;

        /* hash keys and put values into Hanson table */
        return table(dig, nondig, myTable, ogDigList, nonDigCount - 1);
}

/* purpose: hash atom keys to table, store dig strings as vals in a list
* params: digit string, non digit string, number of lines in file, table
*          to hash into, list to store vals for hash tabe, length of non
           dig string
* returns: the master injection sequence 
*/
char *table (char *dig, char *nondig, Table_T myTable, List_T 
                *ogDigList, int nonDigCount) 
{   
        /* initialization */
        const char *nonDigAtom;
        nonDigAtom = Atom_new(nondig, nonDigCount);

        /* hash it*/
        char *tableVal = Table_put(myTable, nonDigAtom, dig);

        /* detect non unique value indicating injection sequence, add digits 
         * to Hanson list, store key as masterSequence
         */
        if (tableVal != NULL) {
                *ogDigList = List_push(*ogDigList, tableVal);
                return nondig;
        }
        free(nondig);
        return NULL;
}

/* purpose: take list of dig strings and make them each their own mini list 
                of ints
* params: pointer to list containing dig strings, length of dig strings
* returns: null pointer
*/
int* listToIntList(List_T *ogDigList) 
{
        *ogDigList = List_reverse(*ogDigList);
        int size = List_length(*ogDigList);
        // (void) lengthOfRow;

        bool printHeader = true;
        for (int i = 0; i < size; i++) {
                char *currString;
                *ogDigList = List_pop(*ogDigList,(void **)&currString);

                List_T currMiniList = List_list(NULL);
                
                stringToIntList(currString, &currMiniList);
                if (i != size - 1) {
                        free(currString);
                }
        
                if (printHeader) {
                        printf("P5\n%i %i\n255\n", List_length(currMiniList), 
                                List_length(*ogDigList ) + 1);
                        printHeader = false;
                }
                int *printMe;
                while (List_length(currMiniList) != 0) {
                        currMiniList = List_pop(currMiniList, (void**)&printMe);
                        putchar(*printMe);
                        free(printMe);
                }

                List_free(&currMiniList);
        }
        return NULL;
}

/* purpose: convert individual string to a mini int list
* params: string to convert, list to store it into
* returns: N/A
*/
void stringToIntList(char *currString, List_T *currMiniList) 
{   

        /*this currLineLength has the length of one string with white spaces*/
        int currLineLength = (int)strlen(currString); 
        int currVal = -1;
        int placeholder = 1;
        int j = 0; //indexing mini array

        for (int i = 0; i < currLineLength; i++) {         
                int digit = 0;
                currVal = -1;
                while (currString[i] != ' ' && currString[i] != '\n') {
                        if (currVal == -1) {
                                currVal = 0;
                        }
                        currVal *= placeholder;
                        currVal += (int)currString[i] - 48;
                        placeholder = 10;
                        digit++;
                        i++;
                }
                
                if (currVal != -1 && currString[i] != EOF) {
                        int *int_p = malloc(sizeof(int_p));
                        *int_p = currVal;
                        *currMiniList = List_push(*currMiniList, int_p);
                        j++;
                }
        }
        *currMiniList = List_reverse(*currMiniList);
}


/* purpose: free memory from table 
* params: table key, table value, closing argument
* returns: N/A
*/
static void vfree(const void *key, void **value, void *cl) 
{
        (void) key;
        (void) cl;
        free(*value);
    
}
