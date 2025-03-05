#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    static cv::Mat LoadImage(const std::string& path) {
        cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
        if (image.empty()) {
            throw std::runtime_error("error while loading image: " + path);
        }
        return image;
    }

    static void SaveImage(const std::string& path, const cv::Mat& image) {
        cv::imwrite(path, image);
    }
};