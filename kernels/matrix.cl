#define BLOCK_SIZE 16

kernel void matrixMul(global float* C, global float* A, global float* B,
	int wA, int wB) {
	int tx = get_global_id(0); // 2D Thread ID x
	int ty = get_global_id(1); // 2D Thread ID y

	// Perform dot-product accumulate into value
	float value = 0;
	for (int k = 0; k < wA; ++k) {
		value += A[ty * wA + k] * B[k * wB + tx];
	}

	// Write to device memory
	C[ty * wA + tx] = value;
}

kernel void matrixMulBlock(global float* C, global float* A, global float* B,
	int wA, int wB) {
	int bx = get_group_id(0); // 2D thread ID x
	int by = get_group_id(1); // 2D thread ID y
	int tx = get_local_id(0); // 2D local ID x
	int ty = get_local_id(1); // 2D local ID y

	// first and last sub-matrix of A for this block
	int aBegin = wA * BLOCK_SIZE * by;
	int aEnd = aBegin + wA - 1;
	int aStep = BLOCK_SIZE;

	// first and last sub-matrix of B for this block
	int bBegin = BLOCK_SIZE * bx;
	int bStep = BLOCK_SIZE * wB;

	float Csub = 0.0;
	// Iterate over all sub-matrices of A and B
	for (int a = aBegin, b = bBegin; a <= aEnd; a += aStep, b += bStep) {

		//static work-group local allocations
		local float As[BLOCK_SIZE][BLOCK_SIZE];
		local float Bs[BLOCK_SIZE][BLOCK_SIZE];

		// Each thread loads one element of the block
		// from global memory
		As[ty][tx] = A[a + wA * ty + tx];
		Bs[ty][tx] = B[b + wB * ty + tx];

		//Barrier to synchronize all threads
		barrier(CLK_LOCAL_MEM_FENCE);
		// Now the local sub-matricies As and Bs are valid

		// Multiply the two sub-matrices. Each thread
		// computes one element of the block sub-matrix.
		for (int k = 0; k < BLOCK_SIZE; ++k)
			Csub += As[ty][k] * Bs[k][tx];

		//Barrier to synchronize all threads before moving on
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
	C[c + wB * ty + tx] = Csub; // write to global memory
}