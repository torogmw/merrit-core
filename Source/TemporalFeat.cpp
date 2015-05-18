#include "TemporalFeat.h"

// Zero Cross Rate (# of cross-zero pairs / # of pairs)
int TemporalFeat::ZCR(const short* buffer, unsigned long length) {
	
	// check if the input arguments are legal
	if (length < 0 || buffer == NULL || (buffer+length-1) == NULL) {
		//printf("Error: illegal arguments.");
		return -1;
	}
	
	// for every adjacent pair, see if it comprises of values with opposite signs
	short counter = 0;
	for (unsigned long i=0; i<length-1; i++) {
		if ((*(buffer+i)) * (*(buffer+i+1)) <= 0) {
			counter ++;
		}
	}
	zcr = (float) counter / (float) length;
	return 0;
}

// Energy = sum[buffer(i)^2]
int TemporalFeat::Energy(const short* buffer, unsigned long length) {
	
	// check if the input arguments are legal
	if (length < 0 || buffer == NULL || (buffer+length-1) == NULL) {
		//printf("Error: illegal arguments.");
		return -1;
	}
	
	// sum up all the square values
	float sum = 0;
	for (unsigned long i=0; i<length; i++) {
		sum += (*(buffer+i)) * (*(buffer+i));
	}
	
	energy = sum;
	return 0;
}