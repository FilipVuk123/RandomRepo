#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "orqa_clock.h"
#include <cuda.h>
 
void VectorAdd(float *a, float *b, int n){
	for(int i = 0; i < n; i++)
		b[i] = a[i] + b[i];
}

int main()
{
	int N = 1000000;
	float *a, *b;
	
	a = (float *)malloc(N * sizeof(float));
	b = (float *)malloc(N * sizeof(float));


	for (int i = 0; i < N; ++i)
	{
		a[i] = 1.0;
		b[i] = 2.0;
	}
	orqa_clock_t clock = orqa_time_now();
	VectorAdd(a, b, N); 
    printf("Time passed: %f\n",orqa_get_time_diff_msec(clock, orqa_time_now()));
	for (int i = 0; i < 10; ++i)
		printf("vector[%d] = %f\n", i, b[i]);

	free(a);
	free(b);
	

	return 0;
}