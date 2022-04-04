#include <stdio.h>
#include <math.h>

extern "C" {
	#include "orqa_clock.h"
}

// Kernel function to add the elements of two arrays
__global__ void add(int n, float *x, float *y)
{
	int index = threadIdx.x;
	int stride = blockDim.x;
	for (int i = index; i < n; i += stride)
		y[i] = x[i] + y[i];
}

int main(void)
{
	int N = 1000000;
	float *x, *y;

	// Allocate Unified Memory – accessible from CPU or GPU
	cudaMallocManaged(&x, N * sizeof(float));
	cudaMallocManaged(&y, N * sizeof(float));

	// initialize x and y arrays on the host
	for (int i = 0; i < N; i++)
	{
		x[i] = 1.0f;
		y[i] = 2.0f;
	}

	// Run kernel on 1M elements on the GPU
	orqa_clock_t clock = orqa_time_now();
	add<<<1, 256>>>(N, x, y);
	printf("Time passed: %f\n",orqa_get_time_diff_msec(clock, orqa_time_now()));
	// Wait for GPU to finish before accessing on host
	cudaDeviceSynchronize();

	for (int i = 0; i < 10; ++i)
		printf("vector[%d] = %f\n", i, y[i]);

	// Free memory
	cudaFree(x);
	cudaFree(y);

	return 0;
}