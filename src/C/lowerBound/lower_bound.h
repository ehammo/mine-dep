/*
 * lower_bound.h
 *
 *  Created on: 10/04/2012
 *      Author: Renato Vimieiro
 */
#ifndef __LOWER_BOUND_H
#define __LOWER_BOUND_H

#include "utils/types.h"

CUDA_WORD* __lowerBound(CUDA_WORD * pos, int sizePos,
						CUDA_WORD * neg, int sizeNeg,
						int setSize);

#endif
