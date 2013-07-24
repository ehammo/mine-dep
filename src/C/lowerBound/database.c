/*
 * database.c
 *
 *  Created on: Sep 14, 2010
 *  Modified on: Mar 12, 2012
 *      Author: Renato Vimieiro
 *  readData assumes that the entire dataset fits in the buffer.
 *  Improvements are required for allowing bigger datasets.
 *  I haven't done that, since this code is for a prototype.
 *  Mind this limitation when using this software.
 */

#include "database.h"
#include "bitarray.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

void readClassData(char * input, unsigned char * dst, int nbSamples){
	FILE * file = fopen(input,"rb");
	int result = fread(dst,sizeof(unsigned char),nbSamples,file);
	assert(result==nbSamples);
	int i;
	for(i=0;i<nbSamples;i++) dst[i]-=48; //char to int 48='0'
	fclose(file);
}

void readData(char * input, Database* rep){
	char * buffer;
	char * end;
	char * cur;
	char * aux;
	size_t size;
	int file;
	unsigned int attsize;
	BitArray * tmp;
	int index;
	unsigned char first;

	file = open(input,O_RDONLY);
	if(file == -1){
		fprintf(stderr,"Couldn't open file %s\n",input);
		exit(EXIT_FAILURE);
	}
    size = lseek (file, 0, SEEK_END);
    lseek(file,0,SEEK_CUR);
	buffer = (char*)mmap(0,size+1,PROT_READ|PROT_WRITE,MAP_PRIVATE,file,(off_t)0);

	if (buffer == MAP_FAILED) {
		fprintf(stderr,"Couldn't map file %s\n",input);
		close(file);
		exit(EXIT_FAILURE);
	}

	cur = buffer;
	end = buffer+size;
	*end = 0;
	index = 0;

	aux = next(end,cur);
	attsize = (unsigned int)atoi(cur);
	rep->objsize = attsize;
	cur = aux;
	aux = next(end,cur);
	*aux = 0;
	attsize = (unsigned int)atoi(cur);
	rep->attsize = attsize;
	rep->data = (BitArray **)malloc(sizeof(BitArray*)*rep->objsize);
//	cur = aux;
//	aux = next(end,cur);
//	*aux = 0;
//	attsize = (unsigned int)atoi(cur);
//	rep->sizePositive = attsize;
	cur = ++aux;

	while(*cur && cur != end){
		tmp = newBitArray(rep->attsize);
		first = 1;
		while(*cur != '\n' && *cur){
			aux = next(end,cur);
			if(*aux == '\n') {
				*aux = 0;
				insertBitArray(tmp,(unsigned int)atoi(cur));
				*aux = '\n';
			} else insertBitArray(tmp,(unsigned int)atoi(cur));
			cur = aux;
		}
		rep->data[index] = tmp;
		index++;
		cur++;
	}

	munmap(buffer,size);
	close(file);
}

char * next(char * end, char * ptr){
	int state;
	char * cur = ptr;
	char now;
	state = 0;
	while(cur != end){
		now = *cur;
		switch(state){
		case 0:
			if(isdigit(now)) cur++;
			else if(now == ' ' || now == '\t' || now == ',') state = 1;
			else if(now == '\n') state = 2;
			else state = -1;
			break;
		case 1:
			*cur = 0;
			return ++cur;
		case 2:
			return cur;
		default:
			fprintf(stderr,"Error while processing input file. Unrecognised char (%c)\n",*cur);
			exit(EXIT_FAILURE);
		}
	}
	return cur;
}
