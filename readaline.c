/*
 *     readaline.c
 *     By Gillian Feder (gfeder01) and Abby Fedor (afedor02), September 2022
 *     filesofpix
 *
 *     This file contains the implementation of the readaline function which
 *     reads in one line from an already opened file, returning the length of
 *     the line. 
 */ 
#include <readaline.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>


/* 
*  Purpose: Retrieve the next line from a previously opened file, and set 
*           datapp to point to the line.
*  Parameters: The file to be read from and a pointer set to the line just read
*  Returns:  length of the line just read from the file, which is the number 
*               of bytes.
*/
size_t readaline(FILE *inputfd, char **datapp) 
{ 
        /*initialization*/
        assert(inputfd != NULL);
        assert(datapp != NULL);
        char *line;
        int capacity = 1000;
        line = (char*)malloc(1000);
        assert(line != NULL);
        char c;
        int count = 0;
        bool stop = false;

        /* reading in char by char */
        while (stop == false && !feof(inputfd)) {
            assert(ferror(inputfd) == 0);
            c = fgetc(inputfd);

            /*expand */
            if (count == capacity - 1 && !feof(inputfd)) {
                    capacity *= 2;
                    line = realloc(line, capacity);
                    assert(line != NULL);
            }
            if (c != EOF) {
                    line[count] = c;
                    count++;
            }

            if (c == '\n' ) {
                    stop = true;
            }

        }
        *datapp = line; /*now datapp points to empty malloced char array*/

        if (feof(inputfd)) {
                free(line);
                *datapp = NULL;
                return 0;
        }
        return count;
}

