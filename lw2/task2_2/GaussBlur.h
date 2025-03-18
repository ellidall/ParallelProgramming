#pragma once

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <thread>
#include <algorithm>
#include "ImageProcessor.h"

class GaussBlur
{
public:
    static const constexpr float GAMMA = 2.2f;

    GaussBlur(int radius, int numThreads) : m_radius(radius), m_numThreads(numThreads)
    {
        m_sigma = GaussBlur::CalculateSigma(radius);
        m_kernel = GaussBlur::GenerateGaussianKernel(m_radius, m_sigma);
    }

    void Apply(cv::Mat& image)
    {
        GaussBlur::ApplyGammaCorrection(image, GaussBlur::GAMMA);

        ApplyGaussianBlur(image, m_kernel, true);
        ApplyGaussianBlur(image, m_kernel, false);

        GaussBlur::ApplyGammaCorrection(image, 1.0f / GaussBlur::GAMMA);
    }

    void SetRadius(int radius)
    {
        m_radius = radius;
        m_kernel = GaussBlur::GenerateGaussianKernel(m_radius, m_sigma);
    }

    //вынести в отдельный класс
    static void ShowInteractiveBlur(const std::string& imagePath)
    {
        cv::Mat image = ImageProcessor::LoadImage(imagePath);
        int radius = 10;
        GaussBlur gaussBlur(radius, 12);

        auto onTrackbar = [](int pos, void* userdata) {
            auto* data = static_cast<std::pair<cv::Mat, GaussBlur*>*>(userdata);
            cv::Mat& img = data->first;
            GaussBlur* gaussBlur = data->second;

            gaussBlur->SetRadius(pos);
            gaussBlur->Apply(img);
            cv::imshow("Gaussian Blur", img);
        };

        cv::namedWindow("Gaussian Blur", cv::WINDOW_NORMAL);
        std::pair<cv::Mat, GaussBlur*> userdata = {image, &gaussBlur};
        cv::createTrackbar("Radius", "Gaussian Blur", nullptr, 500, onTrackbar, &userdata);
        onTrackbar(radius, &userdata);
        cv::waitKey(0);
    }

private:
    int m_radius;
    float m_sigma;
    int m_numThreads;
    std::vector<float> m_kernel;

    static float CalculateSigma(int radius)
    {
        return 0.3f * (static_cast<float>(radius - 1) * 0.5f - 1) + 0.8f;
    }

    static std::vector<float> GenerateGaussianKernel(int radius, float sigma)
    {
        std::vector<float> kernel(2 * radius + 1);
        float sum = 0.0f;

        for (int i = -radius; i <= radius; ++i)
        {
            kernel[i + radius] = std::exp(static_cast<float>(-(i * i)) / (2.0f * sigma * sigma));
            sum += kernel[i + radius];
        }

        for (float& k : kernel)
        {
            k /= sum;
        }

        return kernel;
    }

    static void ApplyGammaCorrection(cv::Mat& image, float gamma)
    {
        // распараллелить
        float invGamma = 1.0f / gamma;
        for (int y = 0; y < image.rows; ++y)
        {
            for (int x = 0; x < image.cols; ++x)
            {
                //выполнив гамма коррекцию лучше сохранить данные во float, чтобы не терять качество
                auto& pixel = image.at<cv::Vec3b>(y, x);
                for (int c = 0; c < 3; ++c)
                {
                    float normalized = static_cast<float>(pixel[c]) / 255.0f;
                    normalized = std::pow(normalized, invGamma);
                    pixel[c] = static_cast<uchar>(std::round(normalized * 255));
                }
            }
        }
    }

    void ApplyGaussianBlur(cv::Mat& image, const std::vector<float>& kernel, bool isHorizontal) const
    {
        int width = image.cols;
        int height = image.rows;
        int halfSize = static_cast<int>(kernel.size()) / 2;
        cv::Mat temp = image.clone();

        auto process = [&](int start, int end) {
            //транспонировать, чтобы было быстрее
            if (isHorizontal)
            {
                for (int y = start; y < end; ++y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        cv::Vec3f sum = cv::Vec3f(0, 0, 0);
                        for (int i = -halfSize; i <= halfSize; ++i)
                        {
                            int ix = std::clamp(x + i, 0, width - 1);
                            sum += kernel[i + halfSize] * cv::Vec3f(image.at<cv::Vec3b>(y, ix));
                        }
                        temp.at<cv::Vec3b>(y, x) = cv::Vec3b(sum);
                    }
                }
            }
            else
            {
                for (int x = start; x < end; ++x)
                {
                    for (int y = 0; y < height; ++y)
                    {
                        cv::Vec3f sum = cv::Vec3f(0, 0, 0);
                        for (int i = -halfSize; i <= halfSize; ++i)
                        {
                            int iy = std::clamp(y + i, 0, height - 1);
                            sum += kernel[i + halfSize] * cv::Vec3f(image.at<cv::Vec3b>(iy, x));
                        }
                        temp.at<cv::Vec3b>(y, x) = cv::Vec3b(sum);
                    }
                }
            }
        };

        std::vector<std::jthread> threads;
        int chunkSize = (isHorizontal ? height : width) / m_numThreads;
        for (int i = 0; i < m_numThreads; ++i)
        {
            int start = i * chunkSize;
            int end = (i == m_numThreads - 1) ? (isHorizontal ? height : width) : (i + 1) * chunkSize;
            threads.emplace_back(process, start, end);
        }

        for (auto& t : threads)
        {
            t.join();
        }

        image = temp.clone();
    }
};
