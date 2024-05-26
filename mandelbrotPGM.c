#define CL_TARGET_OPENCL_VERSION 300
#define _CRT_SECURE_NO_WARNINGS 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <CL/cl.h>

#include "helper.h"

void renderMandelbrot(cl_uchar* canvas, cl_int width, cl_int height, cl_int precision) {
	cl_int status;

	// grab OpenCL context
	cl_device_id device;
	cl_context context = initialiseContext(&device);
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &status);

	// compile kernel
	cl_kernel kernel = compileKernelFromFile("kernel.cl", "renderMandelbrot", context, device);

	// allocate memory on GPU
	cl_mem canvasBuffer = clCreateBuffer(
		context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
		width * height * sizeof(cl_uchar), canvas, &status
	);
	clSetKernelArg(kernel, 0, sizeof(cl_mem), &canvasBuffer);
	clSetKernelArg(kernel, 1, sizeof(cl_int), &precision);

	const size_t GLOBAL_WG_SIZE[2] = { width, height };

	// dispatch workgroups
	status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, GLOBAL_WG_SIZE, NULL, 0, NULL, NULL);
	if (status != CL_SUCCESS) {
		printf("%i Failed to enqueue NDRangeKernel\n", status);
		exit(EXIT_FAILURE);
	}
	status = clEnqueueReadBuffer(
		queue, canvasBuffer, CL_TRUE, 0,
		width * height * sizeof(cl_uchar), canvas, 0, NULL, NULL
	);
	if (status != CL_SUCCESS) {
		printf("%i Failed to enqueue ReadBuffer\n", status);
		exit(EXIT_FAILURE);
	}

	// free OpenCL resources
	clReleaseMemObject(canvasBuffer);
	clReleaseCommandQueue(queue);
	clReleaseKernel(kernel);
	clReleaseDevice(device);
	clReleaseContext(context);
}

int main(int argc, char** argv) {
	// extract arguments
	if (argc != 4) {
		printf("Incorrect Usage:\nmandelbrot_generator [width] [height] [precision]\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 1; i < 4; i++) {
		int j = 0;
		while (argv[i][j] != '\0') {
			if (!isdigit(argv[i][j])) {
				printf("Invalid Argument Specified:\narguments must be positive integers\n");
				exit(EXIT_FAILURE);
			}
			j++;
		}
	}

	const cl_int WIDTH = atoi(argv[1]);
	const cl_int HEIGHT = atoi(argv[2]);
	const cl_int PRECISION = atoi(argv[3]);

	// allocate memory
	cl_uchar* canvas = calloc(WIDTH * HEIGHT, sizeof(cl_uchar));

	// render mandelbrot on GPU
	renderMandelbrot(canvas, WIDTH, HEIGHT, PRECISION);

	// write to pgm binary file
	FILE* image = fopen("mandelbrot.pgm", "wb");
	fprintf(image, "P5\n%d %d\n255\n", WIDTH, HEIGHT);
	fwrite(canvas, sizeof(cl_uchar), WIDTH * HEIGHT, image);

	fclose(image);
	free(canvas);

	return 0;
}