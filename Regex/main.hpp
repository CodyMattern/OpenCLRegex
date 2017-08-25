#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "CLUtil.hpp"
using namespace appsdk;
using namespace std;

#define MAX_SOURCE_SIZE (0x100000)
#define HEAP_SIZE (1000)
#define SAMPLE_VERSION "AMD-APP-SDK-v3.0.130.2"
#define OCL_COMPILER_FLAGS  "CalcPie_OclFlags.txt"

typedef struct HeapStruct {
	cl_uint next;
	cl_uchar heaper[1000];
} myHeap;

class OpenCL
{
	cl_uint seed;      /**< Seed value for random number generation */
	cl_double setupTime;      /**< Time for setting up OpenCL */
	cl_double kernelTime;      /**< Time for kernel execution */
	cl_uint length;     /**< length of the input array */
	cl_context context;      /**< CL context */
	cl_device_id *devices;      /**< CL device list */
	cl_char * pattern;
	myHeap * mh;
	cl_mem patternBuffer;      /**< CL memory buffer */
	cl_mem heapBuffer;      /**< CL memory buffer */
	cl_command_queue commandQueue;      /**< CL command queue */
	cl_program program;      /**< CL program  */
	cl_kernel RegexKernel;      /**< CL kernel */
	int iterations;      /**< Number of iterations for kernel execution */
	SDKDeviceInfo deviceInfo;/**< Structure to store device information*/
	KernelWorkGroupInfo kernelInfo;/**< Structure to store kernel related info */

	SDKTimer *sampleTimer;      /**< SDKTimer object */
	cl_uint stages;

public:
	CLCommandArgs   *sampleArgs;   

	OpenCL():
		seed(123),
		setupTime(0),
		kernelTime(0),
		devices(NULL),
		iterations(1)
	{
		sampleArgs = new CLCommandArgs();
		sampleTimer = new SDKTimer();
		sampleArgs->sampleVerStr = SAMPLE_VERSION;
		sampleArgs->flags = OCL_COMPILER_FLAGS;
		mh = new myHeap();
	}

	/**
	*******************************************************************************
	* @fn Destructor
	* @brief Cleanup the member objects.
	*******************************************************************************
	*/
	~OpenCL()
	{
		FREE(devices);
	}

	/**
	*******************************************************************************
	* @fn setupCalcPie
	* @brief Allocate and initialize host memory array with random values
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int setupRegex();

	/**
	*******************************************************************************
	* @fn setupCL
	* @brief OpenCL related initialisations. Set up Context, Device list,
	*        Command Queue, Memory buffers Build CL kernel program executable.
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int setupCL();

	/**
	*******************************************************************************
	* @fn runCLKernels
	* @brief Set values for kernels' arguments, enqueue calls to the kernels
	*        on to the command queue, wait till end of kernel execution.
	*        Get kernel start and end time if timing is enabled.
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int runCLKernels();

	/**
	*******************************************************************************
	* @fn printStats
	* @brief Override from SDKSample. Print sample stats.
	*******************************************************************************
	*/
	void printStats();

	/**
	*******************************************************************************
	* @fn initialize
	* @brief Override from SDKSample. Initialize command line parser, add custom options.
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int initialize();

	/**
	*******************************************************************************
	* @fn genBinaryImage
	* @brief Override from SDKSample, Generate binary image of given kernel
	*        and exit application.
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int genBinaryImage();



	/**
	*******************************************************************************
	* @fn setup
	* @brief Override from SDKSample, adjust width and height
	*        of execution domain, perform all sample setup
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int setup();

	/**
	*******************************************************************************
	* @fn run
	* @brief Run OpenCL FastWalsh Transform. Override from SDKSample.
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int run();

	/**
	*******************************************************************************
	* @fn cleanup
	* @brief Cleanup memory allocations. Override from SDKSample.
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int cleanup();

	/**
	*******************************************************************************
	* @fn verifyResults
	* @brief Verify against reference implementation
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int verifyResults();

	/**
	*******************************************************************************
	* @fn mapBuffer
	* @brief A common function to map cl_mem object to host
	*
	* @param[in] deviceBuffer : Device buffer
	* @param[out] hostPointer : Host pointer
	* @param[in] sizeInBytes : Number of bytes to map
	* @param[in] flags : map flags
	*
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	template<typename T>
	int mapBuffer(cl_mem deviceBuffer, T* &hostPointer, size_t sizeInBytes,
		cl_map_flags flags);

	/**
	*******************************************************************************
	* @fn unmapBuffer
	* @brief A common function to unmap cl_mem object from host
	*
	* @param[in] deviceBuffer : Device buffer
	* @param[in] hostPointer : Host pointer
	*
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int unmapBuffer(cl_mem deviceBuffer, void* hostPointer);

	/**
	*******************************************************************************
	* @fn runGroupKernel
	* @brief Run group prefixsum CL kernel. The kernel make prefix sum on individual work groups.
	*
	* @param[in] offset : Distance between two consecutive index.
	*
	* @return SDK_SUCCESS on success and SDK_FAILURE on failure
	*******************************************************************************
	*/
	int runRegexKernel();

};
#endif
