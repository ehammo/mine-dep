/*
 * lower_bound.cu
 *
 *  Created on: 10/04/2012
 *      Author: Renato Vimieiro
 */

#include "lower_bound.h"
#include <math.h>
#include <pthread.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysinfo.h>

#include "utils/types.h"

#define MAX_THREAD 256
#define MIN_THREAD_THRESHOLD 100000
#define CPU_THREAD 9999

/********************************************************************
 *																	*
 *	Declarations													*
 *																	*
 ********************************************************************/
 
typedef struct __threadArgs_t {
	CUDA_WORD * pos;
	CUDA_WORD * neg;
	CUDA_WORD * res;
	int setSize;
	int sizeNeg;
	int sizePos;
	int device;
} threadArgs_t;

void* pthreadExecuteDifferenceHost(void *);

inline int loadBalancer(pthread_t ** __threads, threadArgs_t ** __args,
		CUDA_WORD * pos, int sizePos, CUDA_WORD * neg, int sizeNeg, int setSize){
    int nbCPUCores = get_nprocs();
	int nbThreads = 1;
//	int totalMem = sizeNeg*setSize*sizeof(CUDA_WORD);
	int i;
	int mysizePos = sizePos;

	if(sizePos*sizeNeg>MIN_THREAD_THRESHOLD) {
		while(mysizePos%nbCPUCores > 0)nbCPUCores--;
		nbThreads = nbCPUCores;
	} else{
		nbCPUCores = 1;
	}

    (*__threads) = (pthread_t*)malloc(sizeof(pthread_t)*nbThreads);
    (*__args) = (threadArgs_t*)malloc(sizeof(threadArgs_t)*nbThreads);

    threadArgs_t * args = *__args;


    int thread =0;

    if(nbCPUCores>0){
    	for(i=0;i<nbCPUCores;i++){
			args[thread].neg = neg;
			args[thread].setSize = setSize;
			args[thread].sizePos = mysizePos/nbCPUCores;
			args[thread].sizeNeg = sizeNeg;
			args[thread].device = CPU_THREAD; //Device is not used for CPU threads
			args[thread].pos = pos;
			pos += mysizePos/nbCPUCores;
			thread++;
    	}
    }

    args = NULL;

//    fprintf(stderr,"CPU cores = %d, GPUs %d, Threads %d\n",nbCPUCores,nbGPUDevices,nbThreads);


	return nbThreads;
}

/********************************************************************
 *																	*
 *	Definitions														*
 *																	*
 ********************************************************************/
 
 CUDA_WORD* __lowerBound(CUDA_WORD * pos, int sizePos,
						CUDA_WORD * neg, int sizeNeg,
						int setSize){
	pthread_t * threads = NULL;
	threadArgs_t * args = NULL;
	int i;
 	pthread_attr_t attr;
 	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


    int nbThreads = loadBalancer(&threads,&args,pos,sizePos,neg,sizeNeg,setSize);

	fprintf(stderr,"\n============================\n__lowerBound\n ");
	fprintf(stderr,"SetSize = %d\n",setSize);
	fprintf(stderr,"SetPos = %d\n",sizePos);
	fprintf(stderr,"SetNeg = %d\n",sizeNeg);
	fprintf(stderr,"Pos = 0x%llX\n",(unsigned long long)pos);
	fprintf(stderr,"Neg = 0x%llX\n",(unsigned long long)neg);
	fprintf(stderr,"=======End of Print=======\n");
	
	
	for(i=0; i< nbThreads;i++){
		if(args[i].device == CPU_THREAD)
			pthread_create(&threads[i],&attr,pthreadExecuteDifferenceHost,(void*)&(args[i]));
	}

	for(i=0; i< nbThreads;i++)
		pthread_join( threads[i], NULL);
	
	CUDA_WORD * result = (CUDA_WORD*) malloc(sizeof(CUDA_WORD)*setSize*sizePos*sizeNeg);
	CUDA_WORD * ptr = result;
	assert(result);
	for(i=0; i<nbThreads; i++){
		memcpy(ptr,args[i].res,sizeof(CUDA_WORD)*setSize*args[i].sizePos*sizeNeg);
		ptr += setSize*args[i].sizePos*sizeNeg;
		free(args[i].res);
		args[i].res = NULL;    	
	}
	
	free(threads);
	free(args);

	return result;
}
 
/*
  Compute differences on a CPU core (sequentially).
*/
void* pthreadExecuteDifferenceHost(void * threadArgs){

	threadArgs_t * myargs = (threadArgs_t *)threadArgs;
	int setSize = myargs->setSize;
	int sizeNeg = myargs->sizeNeg;
	int sizePos = myargs->sizePos;

	int i,j;
		
	CUDA_WORD * res = (CUDA_WORD*)malloc(sizeof(CUDA_WORD)*setSize*sizeNeg*sizePos);		
	CUDA_WORD * pos = myargs->pos;
	CUDA_WORD * neg = myargs->neg;
	CUDA_WORD * aux_res = res;
//	CUDA_WORD * aux_pos;
	CUDA_WORD * end_pos = pos + (setSize*sizePos);
//	CUDA_WORD * end_neg = neg + (setSize*sizeNeg);
	
	fprintf(stderr,"\n============================\npthreadExecuteDifferenceHost\n ");
	fprintf(stderr,"SetSize = %d\n",setSize);
	fprintf(stderr,"SetPos = %d\n",sizePos);
	fprintf(stderr,"SetNeg = %d\n",sizeNeg);
	fprintf(stderr,"Pos = 0x%llX\n",(unsigned long long)pos);
	fprintf(stderr,"Neg = 0x%llX\n",(unsigned long long)neg);
	fprintf(stderr,"=======End of Print=======\n");


	while(pos < end_pos){
		for(i=0; i < sizeNeg; i++){
			for(j=0;j<setSize;j++)
				aux_res[j] = neg[i*setSize+j] & ~pos[j];
			aux_res+=setSize;	
		}
		pos+=setSize;
	}	
	
	myargs->res = res;
	pthread_exit(NULL);
}
