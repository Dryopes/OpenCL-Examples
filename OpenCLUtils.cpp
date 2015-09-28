#include "OpenCLUtils.h"

void Close() {
	std::cout << "Press any key to quit" << std::endl;
	std::cin.ignore();
	std::cin.get();
	std::exit(1);
}

void CheckError(cl_int error, std::string name)
{
	using namespace std;

	if (error != CL_SUCCESS) {
		cerr << (name != "" ? name + ": " : "") + GetOpenCLErrorInfo(error) << std::endl;
		Close();
	}
}

std::string GetPlatformName(cl::Platform platform)
{
	size_t size = 0;
	cl::STRING_CLASS result;
	CheckError(platform.getInfo(CL_PLATFORM_NAME, &result));

	return result;
}

std::string GetDeviceName(cl_device_id id)
{
	size_t size = 0;
	clGetDeviceInfo(id, CL_DEVICE_NAME, 0, nullptr, &size);

	std::string result;
	result.resize(size);
	clGetDeviceInfo(id, CL_DEVICE_NAME, size,
		const_cast<char*> (result.data()), nullptr);

	return result;
}

std::string LoadKernel(const char* name)
{
	std::ifstream in(name);
	std::string result(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	return result;
}

cl::Platform ChoosePlatform() {
	std::vector<cl::Platform> platforms;
	cl::Platform platform;
	cl::Platform::get(&platforms);

	if (platforms.size() == 0) {
		std::cerr << "No OpenCL platforms found!" << std::endl;
		Close();
	}
	else if (platforms.size() == 1) {
		std::cout << "One platform (" + GetPlatformName(platforms[0]) + "), choosing it per default" << std::endl;
		return platforms[0];
	}

	std::cout << "Found " << platforms.size() << " platforms" << std::endl;
	for (cl_uint i = 0; i < platforms.size(); ++i) {
		std::cout << "\t (" << i << ") : " << GetPlatformName(platforms[i]) << std::endl;
	}

	cl_uint index = -1;
	while (index >= platforms.size()) {
		std::cout << "Choose a platform id:" << std::endl;
		std::cin >> index;
	}

	return platforms[index];
}

cl::Device ChooseDevice(cl::Platform platform) {
	std::vector<cl::Device> devices;

	CheckError(platform.getDevices(CL_DEVICE_TYPE_ALL, &devices));
	if (devices.size() == 0) {
		std::cerr << "No devices found on the chosen platform" << std::endl;
		Close();
	}
	else if (devices.size() == 1) {
		std::cout << "One device found (" + GetDeviceName(devices[0]()) + "), choosing it per default" << std::endl;
		return devices[0];
	}

	std::cout << "Found " << devices.size() << " devices" << std::endl;
	for (cl_uint i = 0; i < devices.size(); ++i) {
		std::cout << "\t (" << i << ") : " << GetDeviceName(devices[i]()) << std::endl;
	}

	cl_uint index = -1;
	while (index >= devices.size()) {
		std::cout << "Choose a device id:" << std::endl;
		std::cin >> index;
	}

	return devices[index];
}