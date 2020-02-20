#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

/*definition of benchmark*/
constexpr int limit = 10000;//integrate from -10000 to 10000
constexpr double dx = 0.0001;//the length of each interval
constexpr int N = limit / dx;//number of interval

#define block 16 //block(s) per gird
#define thread 32 //thread(s) per block
#define stride gridDim.x * blockDim.x

using namespace std;

/*since sinc funtion is even, only the value of sinc with x from 0 to 10000 is calculated, the sum will be doubled to get the final result */
__global__ void fx(double* d_f) {
	int Idx = threadIdx.x + blockIdx.x * blockDim.x;
	for (int i = Idx;i < N;i += stride) {
		double x = (i + 0.5) * dx;
		d_f[i] = sin(M_PI * x) / (M_PI * x);
	}
}

int main() {
	cudaDeviceSynchronize();//synchronise CPU with GPU
	clock_t start = clock();//definition of timer
	
	double* f = new double[N];
	double* d_f;
	cudaMalloc(&d_f, N * sizeof(double));
	fx<<<block, thread>>>(d_f);
	cudaMemcpy(f, d_f, N * sizeof(double), cudaMemcpyDeviceToHost);
	double sum = 0;
	for (int i = 0;i < N;i++) {
		sum += f[i];
	}
	cudaFree(d_f);
	delete[] f;

	cudaDeviceSynchronize();//synchronise CPU with GPU
	clock_t end = clock();

	/*the sum is doubled to get the final result*/
	cout << "the result is:" << 2 * sum * dx << "\n";
	cout << "the runtime is:" << end - start << "ms" << "\n";
	system("pause");
}
