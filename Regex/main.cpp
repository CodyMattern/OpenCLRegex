#include "main.hpp"


int OpenCL::setupRegex()
{
	/*const char pat[] = "a*(b|cd)*";
	length = strlen(pat);
	cl_uint sizeBytes = length * sizeof(cl_char);
	pattern = (cl_char *)malloc(sizeBytes);
	CHECK_ALLOCATION(pattern, "Failed to allocate host memory. (input)");

	for (int i = 0; i < length; i++) {
		pattern[i] = pat[i];
	}*/

	return SDK_SUCCESS;
}

int OpenCL::genBinaryImage()
{
	bifData binaryData;
	binaryData.kernelName = std::string("NFA.cl");
	binaryData.flagsStr = std::string("");
	if (sampleArgs->isComplierFlagsSpecified())
	{
		binaryData.flagsFileName = std::string(sampleArgs->flags.c_str());
	}
	binaryData.binaryName = std::string(sampleArgs->dumpBinary.c_str());
	int status = generateBinaryImage(binaryData);
	return status;
}

int OpenCL::setupCL(void)
{
	cl_int status = 0;
	cl_device_type dType;

	if (sampleArgs->deviceType.compare("cpu") == 0)
	{
		dType = CL_DEVICE_TYPE_CPU;
	}
	else //deviceType = "gpu"
	{
		dType = CL_DEVICE_TYPE_GPU;
		if (sampleArgs->isThereGPU() == false)
		{
			std::cout << "GPU not found. Falling back to CPU device" << std::endl;
			dType = CL_DEVICE_TYPE_CPU;
		}
	}

	// Get platform
	cl_platform_id platform = NULL;
	int retValue = getPlatform(platform, sampleArgs->platformId,
		sampleArgs->isPlatformEnabled());
	CHECK_ERROR(retValue, SDK_SUCCESS, "getPlatform() failed");

	// Display available devices.
	retValue = displayDevices(platform, dType);
	CHECK_ERROR(retValue, SDK_SUCCESS, "displayDevices() failed");

	// If we could find our platform, use it. Otherwise use just available platform.
	cl_context_properties cps[3] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)platform,
		0
	};

	context = clCreateContextFromType(
		cps,
		dType,
		NULL,
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateContextFromType failed.");

	status = getDevices(context, &devices, sampleArgs->deviceId,
		sampleArgs->isDeviceIdEnabled());
	CHECK_ERROR(status, SDK_SUCCESS, "getDevices() failed");

	//Set device info of given cl_device_id
	status = deviceInfo.setDeviceInfo(devices[sampleArgs->deviceId]);
	CHECK_ERROR(status, SDK_SUCCESS, "SDKDeviceInfo::setDeviceInfo() failed");

	//check 2.x compatibility
	bool check2_x = deviceInfo.checkOpenCL2_XCompatibility();

	if (!check2_x)
	{
		OPENCL_EXPECTED_ERROR("Unsupported device! Required CL_DEVICE_OPENCL_C_VERSION 2.0 or higher");
	}

	// Create command queue
	cl_queue_properties prop[] = { 0 };
	commandQueue = clCreateCommandQueueWithProperties(context,
		devices[sampleArgs->deviceId],
		prop,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");



	// create a CL program using the kernel source
	buildProgramData buildData;
	buildData.kernelName = std::string("NFA.cl");
	buildData.devices = devices;
	buildData.deviceId = sampleArgs->deviceId;
	buildData.flagsStr = std::string("");

	if (sampleArgs->isLoadBinaryEnabled())
	{
		buildData.binaryName = std::string(sampleArgs->loadBinary.c_str());
	}

	if (sampleArgs->isComplierFlagsSpecified())
	{
		buildData.flagsFileName = std::string(sampleArgs->flags.c_str());
	}

	retValue = buildOpenCLProgram(program, context, buildData);
	CHECK_ERROR(retValue, SDK_SUCCESS, "buildOpenCLProgram() failed");

	// get a kernel object handle for a kernel with the given name
	RegexKernel = clCreateKernel(program, "Regex", &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel::calc_pie_kernel failed.");

	/* get default work group size */
	status = kernelInfo.setKernelWorkGroupInfo(RegexKernel,
		devices[sampleArgs->deviceId]);
	CHECK_ERROR(status, SDK_SUCCESS, "setKErnelWorkGroupInfo() failed");
	
	patternBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_WRITE,
		sizeof(pattern),
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (randomBuffer)");

	heapBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_WRITE,
		sizeof(mh),
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clCreateBuffer failed. (insideBuffer)");

	return SDK_SUCCESS;
}

int OpenCL::runCLKernels()
{
	size_t dataSize = length;
	size_t localThreads = 2;//kernelInfo.kernelWorkGroupSize;
	size_t globalThreads = dataSize;
	cl_event writeEvt;
	pattern = pattern;
	// Set appropriate arguments to the kernel
	// 1st argument to the kernel - randomBuffer
	int status = clEnqueueWriteBuffer(
		commandQueue,
		patternBuffer,
		CL_FALSE,
		0,
		sizeof(pattern),
		pattern,
		0,
		NULL,
		&writeEvt);

	status = clSetKernelArg(
		RegexKernel,
		0,
		sizeof(cl_mem),
		&patternBuffer);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.(randomBuffer)");

	status = clEnqueueWriteBuffer(
		commandQueue,
		heapBuffer,
		CL_FALSE,
		0,
		sizeof(mh),
		&mh,
		0,
		NULL,
		&writeEvt);

	status = clSetKernelArg(
		RegexKernel,
		1,
		sizeof(cl_mem),
		&heapBuffer);
	CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.(randomBuffer)");


	status = waitForEventAndRelease(&writeEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvt) Failed");

	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.(randomBuffer)");

	// Enqueue a kernel run call
	cl_event ndrEvt;
	status = clEnqueueNDRangeKernel(
		commandQueue,
		RegexKernel,
		1,
		NULL,
		&globalThreads,
		&localThreads,
		0,
		NULL,
		&ndrEvt);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");

	status = waitForEventAndRelease(&ndrEvt);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(ndrEvt) Failed");

	return SDK_SUCCESS;
}

int OpenCL::initialize()
{
	cl_char pat[] = "a*(b|cd)*";
	length = strlen((char *)pat) - 1;
	pattern = (cl_char *)malloc(sizeof(cl_char)*length);
	for (int i = 0; pat[i]; i++) {
		pattern[i] = pat[i];
	}

	// Call base class Initialize to get default configuration
	if (sampleArgs->initialize() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	Option* array_length = new Option;
	CHECK_ALLOCATION(array_length, "Memory allocation error. (array_length)");

	array_length->_sVersion = "x";
	array_length->_lVersion = "length";
	array_length->_description = "Length of the input array";
	array_length->_type = CA_ARG_INT;
	array_length->_value = &length;
	sampleArgs->AddOption(array_length);
	delete array_length;

	Option* num_iterations = new Option;
	CHECK_ALLOCATION(num_iterations, "Memory allocation error. (num_iterations)");

	num_iterations->_sVersion = "i";
	num_iterations->_lVersion = "iterations";
	num_iterations->_description = "Number of iterations for kernel execution";
	num_iterations->_type = CA_ARG_INT;
	num_iterations->_value = &iterations;

	sampleArgs->AddOption(num_iterations);
	delete num_iterations;

	return SDK_SUCCESS;
}

int OpenCL::setup()
{
	int status;
	int timer = sampleTimer->createTimer();
	sampleTimer->resetTimer(timer);
	sampleTimer->startTimer(timer);

	status = setupCL();
	if (status != SDK_SUCCESS)
	{
		return status;
	}

	sampleTimer->stopTimer(timer);
	setupTime = (cl_double)sampleTimer->readTimer(timer);

	if (setupRegex() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	// Move data host to device
	cl_event writeEvtX;
	cl_event writeEvtY;

	status = clEnqueueWriteBuffer(
		commandQueue,
		patternBuffer,
		CL_TRUE,
		0,
		sizeof(pattern),
		pattern,
		0,
		NULL,
		&writeEvtX);

	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.(randomBuffer)");

	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");

	status = waitForEventAndRelease(&writeEvtX);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvtX) Failed");

	status = clEnqueueWriteBuffer(
		commandQueue,
		heapBuffer,
		CL_TRUE,
		0,
		sizeof(mh),
		&mh,
		0,
		NULL,
		&writeEvtY);

	CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.(randomBuffer)");


	status = clFlush(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFlush failed.(commandQueue)");

	status = waitForEventAndRelease(&writeEvtY);
	CHECK_ERROR(status, SDK_SUCCESS, "WaitForEventAndRelease(writeEvtY) Failed");

	return SDK_SUCCESS;
}


int OpenCL::run()
{
	int status = 0;

	//warm up run
	if (runCLKernels() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	std::cout << "Executing kernel for " << iterations
		<< " iterations" << std::endl;
	std::cout << "-------------------------------------------" << std::endl;

	int timer = sampleTimer->createTimer();
	sampleTimer->resetTimer(timer);
	sampleTimer->startTimer(timer);

	for (int i = 0; i < iterations; i++)
	{
		// Arguments are set and execution call is enqueued on command buffer
		if (runCLKernels() != SDK_SUCCESS)
		{
			return SDK_FAILURE;
		}
	}

	sampleTimer->stopTimer(timer);
	kernelTime = (double)(sampleTimer->readTimer(timer));

	return SDK_SUCCESS;
}

/*int OpenCL::verifyResults()
{
	int status = SDK_SUCCESS;

	if (sampleArgs->verify)
	{
		// Read the device output buffer
		cl_float pieValue, gpuPie;
		cl_int *gpuInsideCount;
		int status = mapBuffer(insideBuffer,
			gpuInsideCount,
			sizeof(cl_int),
			CL_MAP_READ);
		CHECK_ERROR(status, SDK_SUCCESS,
			"Failed to map device buffer.(resultBuf)");
		gpuPie = (cl_float)(*gpuInsideCount * 4) / length;

		status = unmapBuffer(insideBuffer, gpuInsideCount);
		CHECK_ERROR(status, SDK_SUCCESS,
			"Failed to unmap device buffer.(resultBuf)");

		// reference implementation
		calcPieCPUReference(&pieValue);

		if (!sampleArgs->quiet)
		{
			std::cout << "GPUInsideCount: " << *gpuInsideCount;
			std::cout << " CPUValue :" << pieValue << " GPUValue: " << gpuPie << std::endl;
		}

		// compare the results and see if they match
		float epsilon = 1e-2f;
		if (::fabs(gpuPie - pieValue) <= epsilon)
		{
			std::cout << "Passed!\n" << std::endl;
			status = SDK_SUCCESS;
		}
		else
		{
			std::cout << "Failed\n" << std::endl;
			status = SDK_FAILURE;
		}

	}

	return status;
}*/

void OpenCL::printStats()
{
	if (sampleArgs->timing)
	{
		std::string strArray[4] =
		{
			"Samples",
			"Setup Time(sec)",
			"Avg. kernel time (sec)",
			"Samples/sec"
		};
		std::string stats[4];
		double avgKernelTime = kernelTime / iterations;

		stats[0] = toString(length, std::dec);
		stats[1] = toString(setupTime, std::dec);
		stats[2] = toString(avgKernelTime, std::dec);
		stats[3] = toString((length / avgKernelTime), std::dec);

		printStatistics(strArray, stats, 4);
	}
}

int OpenCL::cleanup()
{
	// Releases OpenCL resources (Context, Memory etc.)
	cl_int status = 0;

	status = clReleaseKernel(RegexKernel);
	CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.(calc_pie_kernel)");

	status = clReleaseProgram(program);
	CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.(program)");

	status = clReleaseMemObject(patternBuffer);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(randomBuffer)");

	status = clReleaseMemObject(heapBuffer);
	CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed.(randomBuffer)");

	status = clReleaseCommandQueue(commandQueue);
	CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.(commandQueue)");

	status = clReleaseContext(context);
	CHECK_OPENCL_ERROR(status, "clReleaseContext failed.(context)");

	// release program resources
	FREE(pattern);
	FREE(mh);

	return SDK_SUCCESS;
}

template<typename T> int OpenCL::mapBuffer(cl_mem deviceBuffer, T* &hostPointer,
	size_t sizeInBytes, cl_map_flags flags)
{
	cl_int status;
	hostPointer = (T*)clEnqueueMapBuffer(commandQueue,
		deviceBuffer,
		CL_TRUE,
		flags,
		0,
		sizeInBytes,
		0,
		NULL,
		NULL,
		&status);
	CHECK_OPENCL_ERROR(status, "clEnqueueMapBuffer failed");

	status = clFinish(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	return SDK_SUCCESS;
}

int OpenCL::unmapBuffer(cl_mem deviceBuffer, void* hostPointer)
{
	cl_int status;
	status = clEnqueueUnmapMemObject(
		commandQueue,
		deviceBuffer,
		hostPointer,
		0,
		NULL,
		NULL);
	CHECK_OPENCL_ERROR(status, "clEnqueueUnmapMemObject failed");

	status = clFinish(commandQueue);
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	return SDK_SUCCESS;
}

int main(int argc, char * argv[])
{
	OpenCL openCL;
	int status = 0;

	// Initialize
	if (openCL.initialize() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	if (openCL.sampleArgs->parseCommandLine(argc, argv) != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	if (openCL.sampleArgs->isDumpBinaryEnabled())
	{
		//GenBinaryImage
		return openCL.genBinaryImage();
	}

	// Setup
	status = openCL.setup();
	if (status != SDK_SUCCESS)
	{
		return status;
	}

	// Run
	if (openCL.run() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	// VerifyResults
	/*if (openCL.verifyResults() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}*/

	// Cleanup
	if (openCL.cleanup() != SDK_SUCCESS)
	{
		return SDK_FAILURE;
	}

	openCL.printStats();
	exit(0);
	return SDK_SUCCESS;
}