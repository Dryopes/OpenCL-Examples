#ifndef _CLPLATFORMS_H_
#define _CLPLATFORMS_H_

#include <vector>
#include <iostream>
#include <string>

#ifdef __APPLE__
#include "OpenCL/opencl.h"
#else
#include "CL/cl.h"
#endif

class CLPlatforms {
private:
	std::vector<cl_platform_id> ids;

public:
	CLPlatforms();

	cl_platform_id get(int index);

	std::string print();
	std::string getName(int index);
};

#endif