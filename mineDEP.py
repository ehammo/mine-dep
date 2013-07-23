#!/usr/bin/python

#########################################################################################
# CENTRE FOR BIOINFORMATICS, BIOMARKER-DISCOVERY & INFORMATION-BASED MEDICINE		    #	
# THE UNIVERSITY UNIVERSITY OF NEWCASTLE						                        #
# University Drive, Callaghan, NSW, 2308, AUSTRALIA					                    #
#										                                            	#
# Script for mining disjunctive emerging patterns                    					#	
# through enumeration of hypergraphs minimal transversals.			                	#
# Created on: 2012/09/05						                                    	#
# Author: Renato Vimieiro				                                				#
#				                                            							#
# License MIT <http://opensource.org/licenses/MIT>                                      #
#########################################################################################

import sys
from tempfile import NamedTemporaryFile as tmpfile
import tempfile
from subprocess import check_call as execute
import subprocess
from multiprocessing import Pool
from subprocess import CalledProcessError
import os
import signal
#import resource

MINEDEP = 'mineDEP_tmp'
SHD="/home/renato/closed_itemsets/shd31/shd"
LOWER_BOUND='/home/renato/workspace/lowerBoundCPU/Release/lowerBoundCPU'

myfiles = []

def handler(signum, frame):
       print >>sys.stderr, "exiting...",sys.exc_info()
       for f in myfiles: f.close()       
       os.kill(os.getpid(),signal.SIGKILL)

#signal.signal(signal.SIGTERM, handler)
#signal.signal(signal.SIGCHLD, signal.SIG_IGN)

def lowerBoundParam (__filein, __fileclass, __fileout):
    PROGRAM=LOWER_BOUND
    return [PROGRAM,__filein,__fileclass,__fileout]

def lowerBound (__filein, __fileclass, __fileout):
    param = lowerBoundParam(__filein,__fileclass,__fileout)
    #p = subprocess.Popen(param,universal_newlines=True,shell=False,stderr=child_stdout, stdout=child_stdout)
    p = subprocess.Popen(param,universal_newlines=True,shell=False)    
    p.wait()

def paramExecSHD_RS(__filein, __fileout):
    PROGRAM = SHD
    ALGO = "0"
    P1 = "-l"
    P2 = "1"
    INPUT = __filein
    OUTPUT = __fileout
    S="-#" #controls number of patterns to output
    S1="50" #will output only 50 first patterns
    returno = [PROGRAM, ALGO, P1, P2, S, S1, INPUT, OUTPUT]
    return returno

def paramExecSHD_DFS(__filein, __fileout):
    PROGRAM = SHD
    ALGO = "D"
    P1 = "-l"
    P2 = "1"
    INPUT = __filein
    OUTPUT = __fileout
    return [PROGRAM, ALGO, P1, P2, INPUT, OUTPUT]


# In case one wants to use MTMiner -- removed due to efficiency constraints
# def paramExecMTMiner(__filein, __fileout):
#     PROGRAM = MTMiner
#     INPUT = __filein
#     return [PROGRAM, INPUT]


def computeOffset(__filename, __nbPos, __nbNeg):
    result = [0 for i in range(0, __nbPos)]
    _bytes = 0
    with open(__filename) as f:
        for i in range(0, __nbPos):
            result[i] = _bytes
            for j in range(0, __nbNeg):
                _bytes += len(f.readline())
    return result
   

def copyBlock(__inputFile, __tmpInput, __nbNeg):
    for index in range(0, __nbNeg):
        line = __inputFile.readline()       
        __tmpInput.write(line)
    __tmpInput.seek(0, 0)    

def mineHypergraph(Input, Output):
    param = paramExec(Input.name, Output.name)
    p = subprocess.Popen(param,shell=False,universal_newlines=True)
    p.wait()
    p.communicate()
    return 0

def borderDiff((filename, blockSize, offset, index)):
    print >>sys.stderr, 'Processing {0} ...'.format(index)
    inputFile = open(fileName)
    tmpInput = tmpfile('w+r', prefix='{1}-input-s{0}-'.format(index,MINEDEP), delete=False)
    tmpOutput = tmpfile('w+r', prefix='{1}-output-s{0}-'.format(index,MINEDEP), delete=False) 
#    tmpOutput = tmpfile('w+r', prefix=MINEDEP) #In case the output is not required (just computing times) uncomment this and comment the prev
    name = tmpOutput.name
    myfiles.append(inputFile)
    myfiles.append(tmpInput)
    myfiles.append(tmpOutput)   
    try:
         inputFile.seek(offset, 0)   
         copyBlock(inputFile, tmpInput, blockSize)
         r = mineHypergraph(tmpInput, tmpOutput)
    except:
        raise
    finally:  
         tmpInput.close()
         inputFile.close()   
         tmpOutput.close()

    myfiles.remove(inputFile)
    myfiles.remove(tmpInput)
    myfiles.remove(tmpOutput)   
   
    print >>sys.stderr, 'Done with {0}!'.format(index)
#    return r 	#In case the output is not required (just computing times) uncomment this and comment the next
    return name

seen = {}   

def copyResults(Input, Output):
    for line in Input:
        if line not in seen:
            Output.write(line)
            seen[line]=True
    Input.close()   
    return None

def classDistribution (__fileclass):
    nbNeg=0
    nbPos=0
    with open(__fileclass) as f:
         line = f.readline().strip()
         nbPos = line.count('1')
         nbNeg = len(line)-nbPos
    return (nbPos,nbNeg)     

def showUsage(prog):
    print >>sys.stderr, 'Usage: {0} <algo: 0 = SHD_RS, 1 = MTMiner, 2 = SHD_DFS> <nbThreads> <input> <class> <output>'.format(prog)
    sys.exit(1)

if __name__ == '__main__':
   # signal.signal(signal.SIGUSR1, handler)
    if len(sys.argv) < 6: showUsage(sys.argv[0])

    tempfile.tempdir = tempfile.mkdtemp(prefix=MINEDEP,dir="/tmp")
    
    functions = [paramExecSHD_RS,paramExecSHD_DFS] #[paramExecSHD_RS,paramExecMTMiner,paramExecSHD_DFS]
    
    paramExec = functions[int(sys.argv[1])]
    nbThreads = int(sys.argv[2])

    inputFileName = sys.argv[3]
    classfile = sys.argv[4]
    outputName = sys.argv[5]
    
    #print >>sys.stderr, sys.argv
    nbPos,nbNeg = classDistribution(classfile)
    
    tmpOutput = tmpfile('w+', prefix=MINEDEP, delete=False)
    fileName = tmpOutput.name
    tmpOutput.close()
    
    try: lowerBound(inputFileName,classfile,fileName)
    except CalledProcessError as error: 
       print >>sys.stderr, 'LowerBound Error: file ({0}), error ({1})'.format(inputFileName,error)
       os.remove(fileName)
       sys.exit(1)
    

    outputFile = open(outputName, 'w')
    offsets = computeOffset(fileName, nbPos, nbNeg)
   
    proc = Pool(processes=nbThreads)
   
    inputs = [(fileName, nbNeg, offsets[i], i) for i in range(0, nbPos)]
    try:	
        results = proc.map(borderDiff, inputs)
        proc.close()        
    except CalledProcessError as error:
        os.remove(fileName)
        outputFile.close()
        print >>sys.stderr, 'SHD Error: file ({0}), error '.format(inputFileName,error)
        sys.exit(1)
    except SystemExit:
         os._exit(signal.SIGTERM)
    except:
        sys.exit(137) 

	#Copying results to output
    for arxiv in results:
        f = open(arxiv)
        copyResults(f, outputFile)
        f.close()
        print arxiv
        os.remove(arxiv)
    
    del seen
    outputFile.close()
