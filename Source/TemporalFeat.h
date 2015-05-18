#ifndef _TEMPORAL_FEAT
#define _TEMPORAL_FEAT

#include "rsrfft.h"
#include <cmath>

class TemporalFeat {
public:
	int ZCR(const short* buffer, unsigned long length);
	int Energy(const short* buffer, unsigned long length);
	TemporalFeat() {
		zcr = -1;
		energy = -1;
	}
	float zcr;
	float energy;
};

#endif