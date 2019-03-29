#include <stdint.h>
#include <stdlib.h>


#include "broodminder.h"

// magic to convert temperature field to degress F

int
bm_f(u_short tr) {
        double c = (double)tr / (double)0x10000 * 165.0 - 40.0;
	int f = c * (9.0/5.0) + 32.0;
	return f;
}


// similar magic for weights

double
bm_w(uint8_t *b) {
	return (*(b+1) * 256.0 + *b - 32767.0)/100.0;
}

