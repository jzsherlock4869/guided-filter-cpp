#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "guided_filter.h"


int main(int argc, char** argv)
{
    if (argc != 2 and argc != 3)
    {
        printf("usage: GuidedFilter <Input_Image_Path> [<Guide_Image_Path>] \n");
        return -1;
    }

    int radius = 3;
    float epsilon = 0.1;

    cv::Mat inputImage;
    inputImage = cv::imread(argv[1], 1);
    if (!inputImage.data)
    {
        printf("No input image data \n");
        return -1;
    }
    // convert to gray
    // cv::cvtColor(inputImage, inputImage, cv::COLOR_BGR2GRAY);
    
    cv::Mat guideImage;
    if (2 == argc)
    {
        guideImage = inputImage.clone();
        // cv::cvtColor(inputImage, guideImage, cv::COLOR_BGR2GRAY);

    }

    else
    {
        assert(3 == argc);
        guideImage = cv::imread(argv[2], 1);
        if (!guideImage.data)
        {
            printf("No guide image data \n");
            return -1;
        }
    }

    int inputChannels = inputImage.channels();
    int guideChannels = guideImage.channels();
    // valid settings:
    //     guide vs. input
    //       1         1     (e.g. gray self-guided smoothing)
    //       3         1     (e.g. alpha matting mask refine)
    //       1         3
    //       3         3     (e.g. color self-guided smoothing)
    assert(3 == guideChannels or 1 == guideChannels);
    std::cout << "input channel: " << inputChannels << " , guide channel: " << guideChannels << std::endl;

    cv::Mat outputImage;
    if (3 == guideChannels)
    {
        outputImage = guidedFilterColor(inputImage, guideImage, radius, epsilon);
    }
    else
    {
        outputImage = guidedFilterGray(inputImage, guideImage, radius, epsilon);
    }

    // cv::namedWindow("Original Image", cv::WINDOW_AUTOSIZE);
    // cv::imshow("Original Image", inputImage);
    // cv::namedWindow("Filtered Image", cv::WINDOW_AUTOSIZE);
    // cv::imshow("Filtered Image", outputImage);
    // cv::waitKey(-1);

    mkdir("./result_dir", 0777);
    cv::imwrite("./result_dir/inputImage.jpg", inputImage);
    cv::imwrite("./result_dir/outputImage.jpg", outputImage);

    return 0;
}
