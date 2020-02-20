#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

	int id, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/*The program implements 'master-slave' pattern so that has to be executed with at least 2 processes*/
	if (size < 2) {
		printf("Must be executed with at least 2 processes!");
		MPI_Finalize();
		return 1;//print error and exit
	}

	/*Initialize the sequence*/
	
	//n is number of elements in the sequence
	//N is number of elements after padded with 0
	int n, N;
	if (id == 0) {
		printf("Enter number of elements:\n");
		scanf("%d", &n);

		//avoid invalid input
		if (n <= 0) {
			printf("Invalid number of elements!\n");
			printf("Enter number of elements:\n");
			scanf("%d", &n);
		}

		//calculate number of elements after padded with 0
		N = 2;
		while (N < n) {
			N *= 2;
		}
	}
	
	MPI_Bcast(&N, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);//broadcast number of elements after padded with 0 to slaves
	
	int* input = (int*)malloc(N * sizeof(int));//each process initializes an array to store the sequence

	//master generates the input sequence
	if (id == 0) {
		memset(input, 0, N * sizeof(int));//initialize the array with 0
		
		//generate n random integers 0~9
		int i;
		for (i = 0;i < n;++i) {
			input[i] = rand() % 10;
		}

		//master print the input sequence
		printf("input:\n");
		for (i = 0;i < n;++i) {
			printf("%d\n", input[i]);
		}
	}

	//index is the index of element to be updated
	//stride is the steps between the elements to be summed
	int index, stride;

	/*Up Phase*/
	for (stride = 1;stride < N;stride *= 2) {
		MPI_Bcast(input, N, MPI_INT, 0, MPI_COMM_WORLD);//first broadcast the updated sequence to slaves

		//distribute the calculation to slaves
		if (id != 0) {
			for (index = id * stride * 2 - 1;index < N;index += (size - 1) * stride * 2) {
				input[index] += input[index - stride];
				MPI_Send(&input[index], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);//send the updated element to master
			}
		}
		//master collect the updates
		else {
			int I;
			for (I = 1;I <= size - 1;++I) {
				for (index = I * stride * 2 - 1;index < N;index += (size - 1) * stride * 2) {
					MPI_Recv(&input[index], 1, MPI_INT, I, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//receive the updated elements from slaves
				}
			}
		}
	}

	/*Down Phase*/
	for (stride = N / 4;stride > 0;stride /= 2) {
		MPI_Bcast(input, N, MPI_INT, 0, MPI_COMM_WORLD);//first broadcast the updated sequence to slaves

		//distribute the calculation to slaves
		if (id != 0) {
			for (index = id * stride * 2 + stride - 1;index < N;index += (size - 1) * stride * 2) {
				input[index] += input[index - stride];
				MPI_Send(&input[index], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);//send the updated element to master
			}
		}
		//master collect the updates
		else {
			int I;
			for (I = 1;I <= size - 1;++I) {
				for (index = I * stride * 2 + stride - 1;index < N;index += (size - 1) * stride * 2) {
					MPI_Recv(&input[index], 1, MPI_INT, I, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//receive the updated elements from slaves
				}
			}
		}
	}

	/*Master print the output sequence*/
	if (id == 0) {
		printf("output:\n");
		int i;
		for (i = 0;i < n;++i) {
			printf("%d\n", input[i]);
		}
	}

	MPI_Finalize();
	return 0;
}
