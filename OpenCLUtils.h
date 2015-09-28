#ifndef _OPENCLUTILS_H_
#define _OPENCLUTILS_H_

#include <iostream>
#include <fstream>

#include "CL\cl.hpp"

void Close();
void CheckError(cl_int error, std::string name = "");

std::string GetOpenCLErrorInfo(cl_int err);
std::string GetOpenCLErrorName(cl_int err);
std::string GetOpenCLErrorDescription(cl_int err);


std::string GetPlatformName(cl::Platform platform);
std::string GetDeviceName(cl_device_id id);

std::string LoadKernel(const char* name);

cl::Platform ChoosePlatform();
cl::Device ChooseDevice(cl::Platform platform);


#endif //_OPENCLUTILS_H_