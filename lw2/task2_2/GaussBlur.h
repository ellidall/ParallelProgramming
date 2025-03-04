#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <cmath>

class GaussBlur {
public:
    static cv::Mat Apply(const cv::Mat& image, int radius) {
        cv::Mat result;
        cv::GaussianBlur(image, result, cv::Size(2 * radius + 1, 2 * radius + 1), 0);
        return result;
    }

private:
    static std::vector<float> GenerateKernel(int radius) {
        int size = 2 * radius + 1;
        std::vector<float> kernel(size);
        float sigma = radius / 2.0f;
        float sum = 0.0f;

        for (int i = 0; i < size; i++) {
            float x = i - radius;
            kernel[i] = std::exp(-0.5f * (x * x) / (sigma * sigma));
            sum += kernel[i];
        }

        for (float &value : kernel) {
            value /= sum;
        }
        return kernel;
    }
};
