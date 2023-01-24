#include <opencv2/opencv.hpp>
#include <stdio.h>

cv::Mat meanFilter(cv::Mat inputImage, int w);
cv::Mat guidedFilterGray(cv::Mat &inputImage, cv::Mat &guideImage, int radius, float epsilon);
cv::Mat guidedFilterColor(cv::Mat &inputImage, cv::Mat &guideImage, int radius, float epsilon);