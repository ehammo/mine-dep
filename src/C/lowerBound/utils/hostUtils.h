/*
 * hostUtils.h
 *
 *  Created on: 20/04/2012
 *      Author: "Renato Vimieiro"
 */

#ifndef HOSTUTILS_H_
#define HOSTUTILS_H_

#include "types.h"
#include "../bitarray.h"

#ifdef __cplusplus
#define PREFIX extern "C"
#else
#define PREFIX
#endif

PREFIX void computeCardinalities(CUDA_WORD * differences, int * cardIndvSets,int cardDiff, int setSize);
PREFIX void printSet(CUDA_WORD * set,int size);
PREFIX void sortIndicesHost (int * cardinalities, int * indices, int size);
PREFIX CUDA_WORD * toLinearRepresentation(BitArray ** collection, const unsigned int size);

#undef PREFIX
#endif /* HOSTUTILS_H_ */
