/*
 * database.h
 *
 *  Created on: Sep 14, 2010
 *  Updated on: Apr 12, 2012
 *      Author: Renato Vimieiro
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <stdio.h>
#include "bitarray.h"


typedef struct __database {
	BitArray ** data;
	unsigned int attsize; //Number of attributes
	unsigned int objsize; //Number of objects
	unsigned int sizePositive; //determines the number of positive samples (there objsize - sizePositive negative samples)
							   // the first sizePositive samples are from class positive.
} Database;

void readData(char * input, Database* rep);
char * next(char * end, char * ptr);
void readClassData(char * input, unsigned char * dst, int nbSamples);
#endif /* DATABASE_H_ */
