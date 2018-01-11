#include <windows.h>
#include <stdio.h>
#include "mt19937.h"

MT19937 mainPrng;

MT19937::MT19937() {
	mti = N + 1;
}

void MT19937::init_genrand(unsigned long s) {
	mt[0] = s & 0xffffffffUL;

	for (mti = 1; mti < N; mti++) {
		mt[mti]  = (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
		mt[mti] &= 0xffffffffUL;
	}
}

void MT19937::init_by_array(unsigned long *init_key, int key_length) {
	int i, j, k;

	init_genrand(19650218UL);
	i = 1; j = 0;

	k = (N > key_length ? N : key_length);

	for (; k; k--) {
		mt[i]  = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1664525UL)) + init_key[j] + j;
		mt[i] &= 0xffffffffUL;
		i++; j++;

		if (i >= N) {
			mt[0] = mt[N - 1];
			i     = 1;
		}

		if (j >= key_length) {
			j = 0;
		}
	}

	for (k = N - 1; k; k--) {
		mt[i]  = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1566083941UL)) - i;
		mt[i] &= 0xffffffffUL;
		i++;

		if (i >= N) {
			mt[0] = mt[N - 1];
			i     = 1;
		}
	}

	mt[0] = 0x80000000UL;
}

unsigned long MT19937::genrand_int32(void) {
	unsigned long        y;
	static unsigned long mag01[2] = { 0x0UL, MATRIX_A };

	if (mti >= N) {
		int kk;

		/*
		   if (mti == N + 1)
		   {
		   init_genrand(5489UL);
		   }
		 */

		for (kk = 0; kk < N - M; kk++) {
			y      = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}

		for (; kk < N - 1; kk++) {
			y      = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}

		y         = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mti = 0;
	}

	y = mt[mti++];

	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680UL;
	y ^= (y << 15) & 0xefc60000UL;
	y ^= (y >> 18);

	return y;
}

/*
   long genrand_int31(void)
   {
    return (long)(genrand_int32() >> 1);
   }
 */

double MT19937::genrand_real1(void) {
	return genrand_int32() * (1.0 / 4294967295.0);
}

void InitializePRNG() {
	SYSTEMTIME st;
	GetSystemTime(&st);
	mainPrng.init_by_array((unsigned long*)&st, sizeof(SYSTEMTIME) / 4);
}

/*
   double genrand_real2(void)
   {
    return genrand_int32() * (1.0 / 4294967296.0);
   }

   double genrand_real3(void)
   {
    return (((double)genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
   }

   double genrand_res53(void)
   {
    unsigned long a = genrand_int32() >> 5, b = genrand_int32() >> 6;
    return(a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
   }
 */

/*
   int main(void)
   {
    int i;
    unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
    init_by_array(init, length);
    printf("1000 outputs of genrand_int32()\n");
    for (i=0; i<1000; i++) {
      printf("%10lu ", genrand_int32());
      if (i%5==4) printf("\n");
    }
    printf("\n1000 outputs of genrand_real2()\n");
    for (i=0; i<1000; i++) {
      printf("%10.8f ", genrand_real2());
      if (i%5==4) printf("\n");
    }
    return 0;
   }
 */
