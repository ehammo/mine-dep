/*
 * main.c
 *
 *  Created on: 09/03/2012
 *      Author: Renato Vimieiro
 */


//General Includes
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <pthread.h>

//Internal Includes
#include "database.h"
#include "lower_bound.h"
#include "utils/hostUtils.h"

#include "utils/types.h"

#define author "Renato Vimieiro"
#define version "0.99"
#define release "2012-03-09"
#define program "lowerBound (compute lowerBound for disjunctive emerging patterns)"

#define BUFFERSIZE 2<<21
long int numberCandidates = 0;
long int totalClosed = 0;
FILE* output;

int numberAttributes;
unsigned char showTIDs;

Database rep;

struct timeval startglobal, start, stop;
double timespent;

int alpha = 1;
int beta = 0;

long int prunedOrderPreserving = 0;
long int prunedThresholds = 0;

void SIG_HANDLER(int sig){
	gettimeofday(&stop,NULL);
	timespent = (stop.tv_sec-start.tv_sec);
	timespent += (stop.tv_usec-start.tv_usec)/1000000.0;
	fprintf(stderr, "Execution interrupted by signal %d\n", sig);
	fprintf(stderr, "Aborting...\n");
	fprintf(stderr, "Found %ld open sets out of %ld candidates so far in %4.2f sec\n",totalClosed,numberCandidates,timespent);
	fclose(output);
	exit(sig);
}


void showIntro(){
	fprintf(stderr,"%s\t(GPL)\tversion %s  %s\t%s\n",program,version,release,author);
}

void showUsage(char * com){
	fprintf(stderr,"\nProgram usage:\n%s <input_file> <class_file> [<output>]\n",com);
	exit(EXIT_FAILURE);
}

void initializeRepository(Database* rep){
	rep->attsize = 0;
	rep->objsize = 0;
}

int main(int argc, char **argv) {

 	int setSize = 0;
 	int sizeNeg = 0;
 	int sizePos = 0;

	signal(SIGINT,SIG_HANDLER);
	signal(SIGTERM,SIG_HANDLER);

	showTIDs = 0;

	output = stdout;
	showIntro();
	if(argc < 3) showUsage(argv[0]);
	if(argc > 3) output = fopen(argv[3],"w+");

	initializeRepository(&rep);

	gettimeofday(&startglobal,NULL);
	gettimeofday(&start,NULL);

	readData(argv[1],&rep);

	gettimeofday(&stop,NULL);


	timespent = (stop.tv_sec-start.tv_sec);
	timespent += (stop.tv_usec-start.tv_usec)/1000000.0;

	fprintf(stderr,"Database read. Time spent [%4.2f (sec)]\n",timespent);

	if(argc>3)	fprintf(stderr,"Printing patterns in [%s]...\n",argv[3]);
	else fprintf(stderr,"Printing patterns... \n");

	int i,j;
//	fprintf(stderr,"\n============================\nDatabase ");
//	for(i=0; i < rep.objsize; i++) showBitArray(stderr,rep.data[i]);
//	fprintf(stderr,"\n=======End of Database=======\n");
	
	CUDA_WORD * rep_linear = toLinearRepresentation(rep.data,rep.objsize);

//	fprintf(stderr,"\n============================\nDatabase Linear ");
//	for(i=0; i < /*rep.objsize*/rep.data[0]->length; i++) fprintf(stderr,"%llu ",rep_linear[i]);
//	fprintf(stderr,"\n=======End of Database=======\n");
	
	unsigned char * __class = (unsigned char*)malloc(sizeof(unsigned char)*rep.objsize);
 	readClassData(argv[2],__class,rep.objsize);

// 	fprintf(stderr,"\n============================\nClasses ");
//	for(i=0; i < rep.objsize; i++) fprintf(stderr,"%u ",(unsigned)__class[i]);
//	fprintf(stderr,"\n=======End of Classes=======\n");
	
	setSize = rep.data[0]->length;
	for(i = 0; i < rep.objsize; i++) sizePos += __class[i];
 	sizeNeg = rep.objsize - sizePos;
	
	fprintf(stderr,"\n============================\nSizes ");
	fprintf(stderr,"SetSize = %d\n",setSize);
	fprintf(stderr,"SetPos = %d\n",sizePos);
	fprintf(stderr,"SetNeg = %d\n",sizeNeg);
	fprintf(stderr,"=======End of Sizes=======\n");

	gettimeofday(&start,NULL);

    CUDA_WORD * h_pos = rep_linear;
    CUDA_WORD * h_neg = rep_linear + (setSize*sizePos);

//	fprintf(stderr,"\n============================\nAddresses ");
//    fprintf(stderr,"Pos = 0x%llX\n",h_pos);
//	fprintf(stderr,"Neg = 0x%llX\n",h_neg);
//	fprintf(stderr,"=======End of Addresses=======\n");

	CUDA_WORD * h_res;
	h_res = __lowerBound(h_pos,sizePos,h_neg,sizeNeg,setSize);
	free(rep_linear);

//	fprintf(stderr,"\n============================\nResults ");
	for(j = 0; j < sizePos*sizeNeg; j++) printSet(h_res+(j*setSize),setSize);
//	fprintf(stderr,"\n=======End of Results=======\n");

	gettimeofday(&stop,NULL);

	timespent = (stop.tv_sec-start.tv_sec);
	timespent += (stop.tv_usec-start.tv_usec)/1000000.0;

	fprintf(stderr,"Patterns discovered. Time spent [%4.2f (sec)]\n",timespent);

	fprintf(stderr,"Done.\n");
	gettimeofday(&stop,NULL);

	timespent = (stop.tv_sec-startglobal.tv_sec);
	timespent += (stop.tv_usec-startglobal.tv_usec)/1000000.0;

//	fprintf(stderr, "Found %ld closed sets out of %ld candidates in %4.2f sec\nPruned %ld non order preserving candidates and %ld by thresholds\n",
//			totalClosed,numberCandidates,timespent,prunedOrderPreserving,prunedThresholds);

	fclose(output);
    for(i=0;i<rep.objsize;i++) free(rep.data[i]);
	free(rep.data);
	free(__class);
	free(h_res);
	return 0;
}
