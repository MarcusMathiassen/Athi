#pragma once

#include "athi_utility.h"

#include <OpenCL/OpenCL.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>

struct OpenCL
{
    int err;                            // error code returned from api calls
    char* kernel_source{nullptr};
    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation
    unsigned int begin;
    unsigned int end;
    bool gpu{true};

    cl_device_id device_id;             // compute device id
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel

    cl_mem input;                       // device memory used for the input array
    cl_mem output;                      // device memory used for the output array

    OpenCL() = default;

    void init()
    {
        if (kernel_source == nullptr)
        {
            std::cout << "Error: OpenCL missing kernel source. Load it before calling init().\n";
            exit(1);
        }

        std::cout << "OpenCL initializing..\n";

        // Connect to a compute device
        //
        err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
        if (err != CL_SUCCESS)
        {
            std::cout << "Error: Failed to create a device group!\n";
        }

        // Create a compute context
        //
        context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
        if (!context)
        {
            std::cout << "Error: Failed to create a compute context!\n";
        }

        // Create a command commands
        //
        commands = clCreateCommandQueue(context, device_id, 0, &err);
        if (!commands)
        {
            std::cout << "Error: Failed to create a command commands!\n";
        }

        // Create the compute program from the source buffer
        //
        program = clCreateProgramWithSource(context, 1, (const char **) &kernel_source, NULL, &err);
        if (!program)
        {
            std::cout << "Error: Failed to create compute program!\n";
        }

        // Build the program executable
        //
        err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            size_t len;
            char buffer[2048];

            std::cout << "Error: Failed to build program executable!\n";
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
            std::cout << buffer << '\n';
            exit(1);
        }

        // Create the compute kernel in the program we wish to run
        //
        kernel = clCreateKernel(program, "hello", &err);
        if (!kernel || err != CL_SUCCESS)
        {
            std::cout << "Error: Failed to create compute kernel!\n";
            exit(1);
        }
    }

    void load_kernel(const char* file)
    {
        read_file(file, &kernel_source);
    }

    ~OpenCL()
    {
        // Shutdown and cleanup
        //
        clReleaseMemObject(input);
        clReleaseMemObject(output);
        clReleaseProgram(program);
        clReleaseKernel(kernel);
        clReleaseCommandQueue(commands);
        clReleaseContext(context);

        std::cout << "OpenCL shutting down..\n";
    }

};
