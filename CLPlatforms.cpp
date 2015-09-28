#include "CLPlatforms.h"

CLPlatforms::CLPlatforms() {
	cl_uint count = 0;
	clGetPlatformIDs(0, nullptr, &count);

	if (count == 0) {
		std::cerr << "No OpenCL platform found" << std::endl;
		std::exit(1);
	}

	ids = std::vector<cl_platform_id>(count);
	clGetPlatformIDs(count, ids.data(), nullptr);
}

cl_platform_id CLPlatforms::get(int index) {
	return index >= 0 && index < ids.size() ? ids[index] : nullptr;
}

std::string CLPlatforms::print() {
	std::string result;
	for (int index = 0; index < ids.size(); index++) {
		result += getName(index) + "\n";
	}
	return result;
}

std::string CLPlatforms::getName(int index)
{
	size_t size = 0;
	clGetPlatformInfo(ids[index], CL_PLATFORM_NAME, 0, nullptr, &size);

	std::string result;
	result.resize(size);
	clGetPlatformInfo(ids[index], CL_PLATFORM_NAME, size,
		const_cast<char*> (result.data()), nullptr);

	return result;
}