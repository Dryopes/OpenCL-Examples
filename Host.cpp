#include <vector>
#include <string>
#include <ctime>
#include <chrono>
#include <limits>
#include <random>

#include "OpenCLUtils.h"

using namespace std::chrono;

/*
* Dingen
*/

std::vector<float> randomMatrix(float min, float max, int width, int height) {
	std::uniform_real_distribution<float> unif(min, max);
	std::random_device rand_dev;          
	std::mt19937 rand_engine(rand_dev()); 
	
	std::vector<float> result(width*height);
	for (int index = 0; index < width*height; index++) {
		result[index] = unif(rand_engine);
	}

	return result;
}

void matrix(cl::Context context, cl::Device device) {
	std::cout << "---Executing kernels/matrix.cl---" << std::endl;

	const int aWidth = 2048;
	const int aHeight = 2048;
	const int bWidth = 2048;
	const int bHeight = 2048;

	cl_int error;

	/*------------------------------------------
	Vectors and Buffers
	*-------------------------------------------*/

	std::cout << "Creating arguments, buffers" << std::endl;

	std::vector<float> aMatrix = randomMatrix(-10.f, 10.f, aWidth, aHeight);
	std::vector<float> bMatrix = randomMatrix(-10.f, 10.f, bWidth, bHeight);
	std::vector<float> cMatrix(aWidth * bHeight);

	cl::Buffer aBuffer(context, aMatrix.begin(), aMatrix.end(), true, false, &error);
	CheckError(error);
	cl::Buffer bBuffer(context, bMatrix.begin(), bMatrix.end(), true, false, &error);
	CheckError(error);
	cl::Buffer cBuffer(context, cMatrix.begin(), cMatrix.end(), false, false, &error);
	CheckError(error);

	/*------------------------------------------
	Kernel & Program
	*-------------------------------------------*/

	std::cout << "Creating kernels" << std::endl;

	CheckError(error);

	cl::Program program = cl::Program(context, LoadKernel("kernels/matrix.cl"), true, &error);
	CheckError(error);

	cl::Kernel kernel = cl::Kernel(program, "matrixMul", &error);
	CheckError(error);
	kernel.setArg<cl::Buffer>(0, cBuffer);
	kernel.setArg<cl::Buffer>(1, aBuffer);
	kernel.setArg<cl::Buffer>(2, bBuffer);
	kernel.setArg<int>(3, aWidth);
	kernel.setArg<int>(4, bWidth);

	cl::Kernel kernel_block = cl::Kernel(program, "matrixMulBlock", &error);
	CheckError(error);
	kernel_block.setArg<cl::Buffer>(0, cBuffer);
	kernel_block.setArg<cl::Buffer>(1, aBuffer);
	kernel_block.setArg<cl::Buffer>(2, bBuffer);
	kernel_block.setArg<int>(3, aWidth);
	kernel_block.setArg<int>(4, bWidth);

	cl::CommandQueue queue = cl::CommandQueue(context, device);

	/*------------------------------------------
	MatrixMul execution
	*-------------------------------------------*/

	std::cout << "Executing MatrixMul" << std::endl;
	steady_clock::time_point start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(aWidth, bHeight)));
	queue.finish();

	auto time = steady_clock::now() - start;
	std::cout << "Finished MatrixMul in " << duration_cast<milliseconds>(time).count() << " ms" << std::endl;

	queue.enqueueReadBuffer(cBuffer, CL_TRUE, 0, sizeof(int) * cMatrix.size(), cMatrix.data());

	/*------------------------------------------
	MatrixMulBlock execution
	*-------------------------------------------*/

	std::cout << "Executing MatrixMulBlock" << std::endl;
	start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel_block, cl::NullRange, cl::NDRange(aWidth, bHeight), cl::NDRange(16, 16)));
	queue.finish();

	time = steady_clock::now() - start;
	std::cout << "Finished MatrixMulBlock in " << duration_cast<milliseconds>(time).count() << " ms" << std::endl;

	std::vector<float> otherc = cMatrix;
	queue.enqueueReadBuffer(cBuffer, CL_TRUE, 0, sizeof(int) * cMatrix.size(), cMatrix.data());


	std::cout << "---Completed kernels/matrix.cl---" <<std::endl << std::endl;
}

void branch(cl::Context context, cl::Device device) {
	std::cout << "---Executing kernels/branch.cl---" << std::endl;

	cl_int error;
	const int testDataSize = 1024 * 1024 * 64;

	/*------------------------------------------
				Vectors and Buffers
	*-------------------------------------------*/

	std::cout << "Creating arguments, buffers" << std::endl;
	
	std::vector<int> neg(testDataSize);
	for (int i = 0; i < testDataSize; i++) {
		neg[i] = (i % 2 == 0) ? -1 : 1;
	}

	cl::Buffer negBuffer(context, neg.begin(), neg.end(), true, false, &error);

	/*------------------------------------------
				Kernel & Program
	*-------------------------------------------*/

	std::cout << "Creating kernels for BRANCH and NOBRANCH" << std::endl;

	CheckError(error);

	cl::Program program = cl::Program(context, LoadKernel("kernels/branch.cl"), true, &error);
	CheckError(error);

	cl::Kernel kernel_branch = cl::Kernel(program, "BRANCH", &error);
	CheckError(error);
	kernel_branch.setArg<cl::Buffer>(0, negBuffer);

	cl::Kernel kernel_nobranch = cl::Kernel(program, "NOBRANCH", &error);
	CheckError(error);
	kernel_nobranch.setArg<cl::Buffer>(0, negBuffer);

	cl::CommandQueue queue = cl::CommandQueue(context, device);

	/*------------------------------------------
				BRANCH execution
	*-------------------------------------------*/

	std::cout << "Executing BRANCH" << std::endl;
	steady_clock::time_point start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel_branch, cl::NullRange, cl::NDRange(testDataSize)));
	queue.finish();

	auto time = steady_clock::now() - start;
	std::cout << "Finished BRANCH in " << duration_cast<milliseconds>(time).count() << " ms" << std::endl;

	/*------------------------------------------
				NOBRANCH execution
	*-------------------------------------------*/

	std::cout << "Executing NOBRANCH" << std::endl;
	start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel_nobranch, cl::NullRange, cl::NDRange(testDataSize)));
	queue.finish();

	time = steady_clock::now() - start;
	std::cout << "Finished NOBRANCH in " << duration_cast<milliseconds>(time).count() << " ms" << std::endl;

	std::cout << "---Completed kernels/branch.cl---" << std::endl << std::endl;
}

void atomic(cl::Context context, cl::Device device, unsigned int size) {
	std::cout << std::endl;
	std::cout << "---Executing kernels/atomic.cl---" << std::endl;

	/*------------------------------------------
		Vectors, Buffers, Program and Queue
	*-------------------------------------------*/

	cl_int error;
	cl::Program program(context, LoadKernel("kernels/atomic.cl"), true, &error);
	CheckError(error, "atomic_program");

	cl::CommandQueue queue(context, device);

	int sum = 0;
	cl::Buffer bufferSum = cl::Buffer(context, CL_MEM_READ_WRITE, 1 * sizeof(float));

	std::vector<int> aVec(size, 1);
	cl::Buffer aBuffer = cl::Buffer(context, aVec.begin(), aVec.end(), true, false, &error);
	CheckError(error, "atomic_abuffer");

	/*------------------------------------------
					Kernels
	*-------------------------------------------*/

	cl::Kernel kernel_atomic1 = cl::Kernel(program, "AtomicSum");
	kernel_atomic1.setArg(0, bufferSum);
	kernel_atomic1.setArg<cl::Buffer>(1, aBuffer);

	cl::Kernel kernel_atomic2 = cl::Kernel(program, "AtomicSum2");
	kernel_atomic2.setArg(0, bufferSum);
	kernel_atomic2.setArg(1, aBuffer);

	cl::Kernel kernel_atomic3 = cl::Kernel(program, "AtomicSum3");
	kernel_atomic3.setArg(0, bufferSum);
	kernel_atomic3.setArg(1, aBuffer);

	/*------------------------------------------
				ATOMIC1 execution
	*-------------------------------------------*/

	//preperations of sum (first argument)
	sum = 0;
	queue.enqueueWriteBuffer(bufferSum, CL_TRUE, 0, 1 * sizeof(int), &sum);

	std::cout << "Executing AtomicSum" << std::endl;
	steady_clock::time_point start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel_atomic1, cl::NullRange, cl::NDRange(size)), "atomic_enqueueNDRangeKernel");
	queue.finish();
	auto time = steady_clock::now() - start;
	
	queue.enqueueReadBuffer(bufferSum, CL_TRUE, 0, 1 * sizeof(int), &sum);
	std::cout << "AtomicSum finished at " << duration_cast<milliseconds>(time).count() << " ms (sum: " << sum << ", should be: " << size << ")" << std::endl;

	/*------------------------------------------
				ATOMIC2 execution
	*-------------------------------------------*/

	//preperations of sum (first argument)
	sum = 0;
	queue.enqueueWriteBuffer(bufferSum, CL_TRUE, 0, 1 * sizeof(int), &sum);

	std::cout << "Executing AtomicSum2" << std::endl;
	start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel_atomic2, cl::NullRange, cl::NDRange(1024 * 1024 * 128), cl::NullRange));
	queue.finish();

	time = steady_clock::now() - start;
	queue.enqueueReadBuffer(bufferSum, CL_TRUE, 0, 1 * sizeof(int), &sum);
	std::cout << "AtomicSum2 finished at " << duration_cast<milliseconds>(time).count() << " ms (sum: " << sum << ", should be: " << size << ")" << std::endl;


	/*------------------------------------------
				ATOMIC3 execution
	*-------------------------------------------*/

	//preperations of sum (first argument)
	sum = 0;
	queue.enqueueWriteBuffer(bufferSum, CL_TRUE, 0, 1 * sizeof(int), &sum);

	std::cout << "Executing AtomicSum3" << std::endl;
	start = steady_clock::now();

	CheckError(queue.enqueueNDRangeKernel(kernel_atomic3, cl::NullRange, cl::NDRange(1024 * 1024 * 128), cl::NullRange));
	queue.finish();

	time = steady_clock::now() - start;
	queue.enqueueReadBuffer(bufferSum, CL_TRUE, 0, 1 * sizeof(int), &sum);
	std::cout << "AtomicSum3 finished at " << duration_cast<milliseconds>(time).count() << " ms (sum: " << sum << ", should be: " << size << ")" << std::endl;
	std::cout << "---Completed kernels/atomic.cl---" << std::endl << std::endl;
}




int main()
{
	cl::Platform platform = ChoosePlatform();
	cl::Device device = ChooseDevice(platform);
	cl::Context context(device);

	std::cout << std::endl;

	matrix(context, device);
	branch(context, device);
	atomic(context, device, 1024*1024*64);

	std::cout << "End Program!" << std::endl;
	std::cin.ignore();
	std::cin.get();
}