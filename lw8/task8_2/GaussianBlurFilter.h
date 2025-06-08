#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>

struct Image {
    int width;
    int height;
    std::vector<cl_float4> pixels;
};

Image LoadPPM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open " + filename);

    std::string magic;
    file >> magic;
    if (magic != "P6") throw std::runtime_error("Unsupported PPM format");

    int width, height, maxval;
    file >> width >> height >> maxval;
    file.get();

    std::vector<unsigned char> data(width * height * 3);
    file.read(reinterpret_cast<char*>(data.data()), data.size());

    Image image;
    image.width = width;
    image.height = height;
    image.pixels.resize(width * height);

    for (int i = 0; i < width * height; ++i) {
        float r = data[i*3] / 255.0f;
        float g = data[i*3 + 1] / 255.0f;
        float b = data[i*3 + 2] / 255.0f;
        image.pixels[i] = { r, g, b, 1.0f };
    }
    return image;
}

void SavePPM(const std::string& filename, const Image& image) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open " + filename);

    file << "P6\n" << image.width << " " << image.height << "\n255\n";

    for (int i = 0; i < image.width * image.height; ++i) {
        unsigned char r = static_cast<unsigned char>(std::fmin(std::fmax(image.pixels[i].s[0], 0.f), 1.f) * 255);
        unsigned char g = static_cast<unsigned char>(std::fmin(std::fmax(image.pixels[i].s[1], 0.f), 1.f) * 255);
        unsigned char b = static_cast<unsigned char>(std::fmin(std::fmax(image.pixels[i].s[2], 0.f), 1.f) * 255);
        file.put(r);
        file.put(g);
        file.put(b);
    }
}

std::string LoadKernelSource(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Cannot open kernel file: " + filename);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::vector<float> CreateGaussianKernel(int radius) {
    std::vector<float> kernel(radius * 2 + 1);
    float sigma = radius / 3.0f;
    float sum = 0.0f;
    for (int i = -radius; i <= radius; ++i) {
        float v = std::exp(-(i*i) / (2.0f * sigma * sigma));
        kernel[i + radius] = v;
        sum += v;
    }
    for (auto& v : kernel) v /= sum;
    return kernel;
}

class GpuGaussianBlur {
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernelHorizontal;
    cl_kernel kernelVertical;
    cl_device_id device;

public:
    GpuGaussianBlur(const std::string& kernelPath) {
        cl_int err;

        cl_platform_id platform;
        err = clGetPlatformIDs(1, &platform, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to get OpenCL platform");

        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to get OpenCL device");

        context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create OpenCL context");

        queue = clCreateCommandQueue(context, device, 0, &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create command queue");

        std::string source = LoadKernelSource(kernelPath);
        const char* src = source.c_str();
        program = clCreateProgramWithSource(context, 1, &src, nullptr, &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create program");

        err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            size_t log_size;
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
            std::vector<char> log(log_size);
            clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
            std::cerr << "Build log:\n" << log.data() << std::endl;
            throw std::runtime_error("Failed to build program");
        }

        kernelHorizontal = clCreateKernel(program, "GaussianBlurHorizontal", &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create kernel GaussianBlurHorizontal");

        kernelVertical = clCreateKernel(program, "GaussianBlurVertical", &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create kernel GaussianBlurVertical");
    }

    ~GpuGaussianBlur() {
        clReleaseKernel(kernelHorizontal);
        clReleaseKernel(kernelVertical);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
    }

    void ApplyGaussianBlur(Image& image, int radius) {
        cl_int err;
        size_t imgSize = image.width * image.height;
        std::vector<float> kernel = CreateGaussianKernel(radius);

        cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(cl_float4) * imgSize, image.pixels.data(), &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create input buffer");

        cl_mem tempBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
                sizeof(cl_float4) * imgSize, nullptr, &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create temp buffer");

        cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                sizeof(cl_float4) * imgSize, nullptr, &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create output buffer");

        cl_mem kernelBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float) * kernel.size(), kernel.data(), &err);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to create kernel buffer");

        // транспонировать и использовать только горизонтальное ядро
        // копировать в локальную память

        err  = clSetKernelArg(kernelHorizontal, 0, sizeof(cl_mem), &inputBuffer);
        err |= clSetKernelArg(kernelHorizontal, 1, sizeof(cl_mem), &tempBuffer);
        err |= clSetKernelArg(kernelHorizontal, 2, sizeof(cl_mem), &kernelBuffer);
        err |= clSetKernelArg(kernelHorizontal, 3, sizeof(int), &radius);
        err |= clSetKernelArg(kernelHorizontal, 4, sizeof(int), &image.width);
        err |= clSetKernelArg(kernelHorizontal, 5, sizeof(int), &image.height);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to set kernelHorizontal arguments");

        size_t globalSize[2] = { (size_t)image.width, (size_t)image.height };
        err = clEnqueueNDRangeKernel(queue, kernelHorizontal, 2, nullptr, globalSize, nullptr, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to enqueue kernelHorizontal");

        err  = clSetKernelArg(kernelVertical, 0, sizeof(cl_mem), &tempBuffer);
        err |= clSetKernelArg(kernelVertical, 1, sizeof(cl_mem), &outputBuffer);
        err |= clSetKernelArg(kernelVertical, 2, sizeof(cl_mem), &kernelBuffer);
        err |= clSetKernelArg(kernelVertical, 3, sizeof(int), &radius);
        err |= clSetKernelArg(kernelVertical, 4, sizeof(int), &image.width);
        err |= clSetKernelArg(kernelVertical, 5, sizeof(int), &image.height);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to set kernelVertical arguments");

        err = clEnqueueNDRangeKernel(queue, kernelVertical, 2, nullptr, globalSize, nullptr, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to enqueue kernelVertical");

        err = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, sizeof(cl_float4) * imgSize, image.pixels.data(), 0, nullptr, nullptr);
        if (err != CL_SUCCESS) throw std::runtime_error("Failed to read output buffer");

        clReleaseMemObject(inputBuffer);
        clReleaseMemObject(tempBuffer);
        clReleaseMemObject(outputBuffer);
        clReleaseMemObject(kernelBuffer);
    }
};
