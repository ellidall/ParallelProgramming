#pragma once

#include <CL/opencl.h>
#include <CL/cl2.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <limits>

class BitonicSorter {
public:
    BitonicSorter() {
        InitializeOpenCL();
    }

    void Sort(std::vector<int>& data) {
        PadToPowerOfTwo(data);
        size_t n = data.size();

        cl::Buffer buffer(m_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * n, data.data());

        for (size_t k = 2; k <= n; k <<= 1) {
            for (size_t j = k >> 1; j > 0; j >>= 1) {
                m_kernel.setArg(0, buffer);
                m_kernel.setArg(1, static_cast<cl_uint>(j));
                m_kernel.setArg(2, static_cast<cl_uint>(k));

                cl::NDRange global(n);
                m_queue.enqueueNDRangeKernel(m_kernel, cl::NullRange, global, cl::NullRange);
                // без ожтданий
                m_queue.finish();
            }
        }

        m_queue.enqueueReadBuffer(buffer, CL_TRUE, 0, sizeof(int) * n, data.data());
    }

private:
    cl::Context m_context;
    cl::Device m_device;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    cl::Kernel m_kernel;
    
    void InitializeOpenCL() {
        std::vector<cl::Platform> m_platforms;
        cl::Platform::get(&m_platforms);
        if (m_platforms.empty()) throw std::runtime_error("No OpenCL platforms found");

        cl::Platform m_platform = m_platforms[0];

        std::vector<cl::Device> m_devices;
        m_platform.getDevices(CL_DEVICE_TYPE_GPU, &m_devices);
        if (m_devices.empty()) throw std::runtime_error("No GPU devices found");

        m_device = m_devices[0];
        m_context = cl::Context(m_device);
        m_queue = cl::CommandQueue(m_context, m_device);

        const char* m_kernelSource = R"(
// код понятнее
        __kernel void bitonicSort(__global int* data, uint j, uint k) {
            uint i = get_global_id(0);
            uint ixj = i ^ j;
            if (ixj > i) {
                if ((i & k) == 0) {
                    if (data[i] > data[ixj]) {
                        int temp = data[i];
                        data[i] = data[ixj];
                        data[ixj] = temp;
                    }
                } else {
                    if (data[i] < data[ixj]) {
                        int temp = data[i];
                        data[i] = data[ixj];
                        data[ixj] = temp;
                    }
                }
            }
        })";

        std::string kernelSourceStr(m_kernelSource);
        cl::Program::Sources m_sources(1, kernelSourceStr);
        m_program = cl::Program(m_context, m_sources);

        if (m_program.build({ m_device }) != CL_SUCCESS) {
            throw std::runtime_error("Error building OpenCL program: " + m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_device));
        }

        m_kernel = cl::Kernel(m_program, "bitonicSort");
    }

    static void PadToPowerOfTwo(std::vector<int>& data) {
        size_t n = data.size();
        size_t pow2 = 1;
        while (pow2 < n) pow2 <<= 1;
        if (pow2 != n) {
            data.resize(pow2, std::numeric_limits<int>::max());
        }
    }
};
