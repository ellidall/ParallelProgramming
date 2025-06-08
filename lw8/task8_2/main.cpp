#include "GaussianBlurFilter.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: gauss.exe input.ppm radius\n";
        return 1;
    }

    try {
        std::string inputFile = argv[1];
        int radius = std::stoi(argv[2]);
        Image image = LoadPPM(inputFile);

        GpuGaussianBlur blur("gaussian_blur.cl");
        blur.ApplyGaussianBlur(image, radius);

        std::string outputFile = "output.ppm";
        SavePPM(outputFile, image);

        std::cout << "Blur applied, saved to " << outputFile << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}