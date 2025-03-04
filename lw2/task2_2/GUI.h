#pragma once
#include <opencv2/opencv.hpp>
#include "GaussBlur.h"
#include "ImageProcessor.h"

class GUI {
public:
    static void ShowInteractiveBlur(const std::string& imagePath) {
        cv::Mat image = ImageProcessor::LoadImage(imagePath);
        int radius = 5;

        auto onTrackbar = [](int pos, void* userdata) {
            auto* img = static_cast<cv::Mat*>(userdata);
            cv::Mat blurred = GaussBlur::Apply(*img, pos);
            cv::imshow("Gaussian Blur", blurred);
        };

        cv::namedWindow("Gaussian Blur");
        cv::createTrackbar("Radius", "Gaussian Blur", &radius, 50, onTrackbar, &image);
        onTrackbar(radius, &image);
        cv::waitKey(0);
    }
};