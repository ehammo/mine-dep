/*
 * hostUtils.c
 *
 *  Created on: 20/04/2012
 *      Author: "Renato Vimieiro"
 */

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <assert.h>
#include "hostUtils.h"

#define CHUNKSIZE 1000

extern FILE * output;
/*
inline unsigned int popcountll(unsigned long long v){
  // unsigned long long v; // count bits set in this (32-bit value)
  unsigned int c; // store the total here
 #define T unsigned long long
  v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
  v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
  v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
  c = (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * 8; // count
#undef T
  return c;
  }*/
//#define popcountll __builtin_popcountll
#include <smmintrin.h>
#define popcountll(a) _mm_popcnt_u64(a)


void computeCardinalities(CUDA_WORD * differences, int * cardIndvSets,int cardDiff, int setSize){

	int i,j, offset;

#pragma omp parallel private(i,j,offset) if(cardDiff > CHUNKSIZE) num_threads(12)
	{
//		fprintf(stderr,"TID %d\n",omp_get_thread_num());
		#pragma omp for schedule(dynamic,CHUNKSIZE) nowait
		  for (i=0; i < cardDiff; i++){
    			offset = i*setSize;
    			cardIndvSets[i]=0;
    			for(j=offset; j < offset+setSize; j++) {
    				cardIndvSets[i]+= popcountll(differences[j]);
    			}
    	  }
	} //parallel OMP
}
#undef CHUNKSIZE

void printSet(CUDA_WORD * set, int size){
	int i = 0, j= 0;
	int count = 0;
//	fprintf(output,"[");
	for(i=0; i < size; i++){
		int nbits = sizeof(CUDA_WORD)<<3;
		if(set[i] > 0) {
			const CUDA_WORD one = 1;
			for(j = 0; j < nbits; j++,count++){
				if( (one<<j) & set[i] )
					fprintf(output,"%d ",count);
			}
		}
		else{
			count+= nbits;
		}
	}
	fseek(output,-1,SEEK_END);
	fprintf(output,"\n");
}
static int * keys;
static int compare (const void * a, const void * b){
	int __a = *((int*)a);
	int __b = *((int*)b);
	return keys[__a] - keys[__b];
}

void sortIndicesHost (int * cardinalities, int * indices, int size){
	int i;
	for(i=0;i<size;i++) indices[i] = i;
	keys = cardinalities;
	qsort(indices,size,sizeof(int),compare);
}

CUDA_WORD * toLinearRepresentation(BitArray ** collection, const unsigned int size){
	assert(collection);
	CUDA_WORD * rep, * iter;
	BitArray ** ptr = collection,
			 ** end = collection + size;
	int lengthArray = collection[0]->length;
	rep = (CUDA_WORD *)malloc(sizeof(CUDA_WORD)*size*lengthArray);
	iter = rep;
	assert(rep);
	while(ptr < end){
		memcpy(iter,(*ptr)->data,sizeof(CUDA_WORD)*lengthArray);
		iter+=lengthArray;
		ptr++;
	}
	return rep;
}
