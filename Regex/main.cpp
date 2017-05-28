#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#define MAX_SOURCE_SIZE (0x100000)
using namespace std;
using namespace cl;


Platform getPlatform() {
	/* Returns the first platform found. */
	std::vector<Platform> all_platforms;
	Platform::get(&all_platforms);

	if (all_platforms.size() == 0) {
		cout << "No platforms found. Check OpenCL installation!\n";
		exit(1);
	}
	return all_platforms[0];
}


Device getDevice(Platform platform, int i, bool display = false) {
	/* Returns the deviced specified by the index i on platform.
	* If display is true, then all of the platforms are listed.
	*/
	std::vector<Device> all_devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if (all_devices.size() == 0) {
		cout << "No devices found. Check OpenCL installation!\n";
		exit(1);
	}

	if (display) {
		for (int j = 0; j<all_devices.size(); j++)
			printf("Device %d: %s\n", j, all_devices[j].getInfo<CL_DEVICE_NAME>().c_str());
	}
	return all_devices[i];
}


int main() {

	const char pattern[] = "(a.(b.(c)))";
	char *results = (char*)malloc(sizeof(pattern));

	Platform default_platform = getPlatform();
	Device default_device = getDevice(default_platform, 1);
	Context context({ default_device });
	Program::Sources sources;
	ifstream infile{ "./Regex.cl" };

	string kernel_code{ istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
	sources.push_back({ kernel_code.c_str(), kernel_code.length() });

	Program program(context, sources);
	if (program.build({ default_device }) != CL_SUCCESS) {
		cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
		exit(1);
	}

	CommandQueue queue(context, default_device);

	std::size_t patSize = sizeof(char) * strlen(pattern);
	Buffer bufferPattern(context, CL_MEM_READ_WRITE, patSize, NULL);
	queue.enqueueWriteBuffer(bufferPattern, CL_TRUE, 0, patSize, pattern);
	
	
	Buffer bufferStack(context, CL_MEM_READ_WRITE, patSize, NULL);
	Buffer bufferResults(context, CL_MEM_READ_WRITE, patSize, NULL);

	Kernel regex = Kernel(program, "PostRegex");
	regex.setArg(0, bufferPattern);
	regex.setArg(1, bufferStack);
	regex.setArg(2, bufferResults);

	NDRange global(12, 1);
	int ret;
	ret = queue.enqueueNDRangeKernel(regex, NullRange, global, NullRange);
	queue.enqueueReadBuffer(bufferResults, CL_TRUE, 0, sizeof(char) * strlen(pattern), results);

	puts(results);

	getchar();

	return 0;
}