#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    static cv::Mat LoadImage(const std::string& path) {
        return cv::imread(path, cv::IMREAD_UNCHANGED);
    }

    static void SaveImage(const std::string& path, const cv::Mat& image) {
        cv::imwrite(path, image);
    }
};