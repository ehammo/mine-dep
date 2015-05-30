mine-dep
========

Contains the source code of the algorithm for mining disjunctive emerging patterns using hypergraphs

Configuration
-------------

After cloning/downloading the source code, make sure binary executable files in */bin*
are correct for your operating system. Please consider recompiling these files.


	cd src/C/shd31
	make
	cd ../lowerBound/Release
	make clean
	make
	cd ../../../../


Input format
------------

The input is composed of two files: (1) one containing the actual data; and (2) one with class labels for each object.
The data file format is the following:

- First line must contain, in this order, the number of objects and attributes separated by ' ' or '\t' or ','
- Each line starting from the second until the number of objects must be a list of attributes that the object contains 
(attributes must be discretized and attribute-value pairs indexed from 0, which corresponds to the set of features
in the dataset as described in the paper).

The objects must be sorted first appearing all positive samples followed by negatives.

The class file contains a single line composed by a sequence of 1s (for each positive sample) followed by a sequence of 0s (for each negative).
There must not be any separator between them.

Here is an example taken from Vimieiro (2012, Table 5.1):

Original dataset

age  | education   | children    | occupation | class
---  | ---------   | --------    | ---------- | -----
30s  | 4           | ≤4          | 1          | pos
30s  | 3           | >4          | 1          | pos
30s  | 3           | >4          | 2          | pos
teen | 3           | ≤4          | 2          | pos
20s  | 2           | ≤4          | 2          | neg
40s  | 1           | >4          | 3          | neg
40s  | 2           | >4          | 3          | neg
40s  | 3           | >4          | 3          | neg


Indexed attribute-value pairs

	teen 0
	20s 1
	30s 2
	40s 3
	e1 4
	e2 5
	e3 6
	e4 7
	>4 8
	≤4 9
	o1 10
	o2 11
	o3 12

Data file

	8 13
	2 7 9 10
	2 6 8 10
	2 6 8 11
	0 6 9 11
	1 5 9 11
	3 4 8 12
	3 5 8 12
	3 6 8 12

Class file

	11110000


References
----------

Vimieiro, R. (2012) Mining disjunctive patterns in biomedical data sets. PhD thesis.
The University of Newcastle, Australia. http://hdl.handle.net/1959.13/936341
