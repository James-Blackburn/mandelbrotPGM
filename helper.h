#define _CRT_SECURE_NO_WARNINGS 

#ifndef HELPER_C_INCLUDED
#define HELPER_C_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

cl_context initialiseContext(cl_device_id* device) {
	// get first platform on system
	cl_int status;
	cl_platform_id platform;
	status = clGetPlatformIDs(1, &platform, NULL);
	if (status != CL_SUCCESS) {
		printf("Could not find an OpenCL platform\n");
		exit(EXIT_FAILURE);
	}

	// get first device on platform
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, device, NULL);
	if (status != CL_SUCCESS) {
		printf("Failed to get an OpenCL compatible GPU\n");
		exit(EXIT_FAILURE);
	}

	// create an OpenCL context
	cl_context context = clCreateContext(NULL, 1, device, NULL, NULL, &status);
	if (status != CL_SUCCESS) {
		printf("Failed to create an OpenCL context\n");
		exit(EXIT_FAILURE);
	}

	return context;
}

cl_kernel compileKernelFromFile(const char* filename, const char* kernelName, cl_context context, cl_device_id device) {
	FILE* fp;
	char* buffer;
	long fileSize;

	// open the file
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Could not open file: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// get file size
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// read in data
	buffer = (char*)malloc(fileSize + 1);
	if (fread(buffer, fileSize, 1, fp) != 1) {
		printf("Error reading file: %s\n", filename);
		exit(EXIT_FAILURE);
	}
	buffer[fileSize] = '\0';

	fclose(fp);

	// create an OpenCL program
	cl_int status;
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&buffer, NULL, &status);
	if (status != CL_SUCCESS) {
		printf("Failed to create program from source: %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// build the program
	status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (status != CL_SUCCESS) {
		printf("Failed to build kernel '%s' from file '%s'\n error code %i.\n", kernelName, filename, status);

		// Provide more information about the nature of the fail.
		size_t logSize;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		char* log = (char*)malloc((logSize) * sizeof(char));

		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
		printf("Build log:\n%s\n", log);

		// Clear up and quit.
		free(log);
		exit(EXIT_FAILURE);
	}

	// compile kernel
	cl_kernel kernel = clCreateKernel(program, kernelName, &status);
	if (status != CL_SUCCESS) {
		printf("Failed to create OpenCL kernel with error code: %i.\n", status);
		exit(EXIT_FAILURE);
	}

	// free data
	clReleaseProgram(program);
	free(buffer);

	return kernel;
}

#endif