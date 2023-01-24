#include "guided_filter.h"

cv::Mat meanFilter(cv::Mat inputImage, int w)
{
    cv::Mat outputImage;
    cv::blur(inputImage, outputImage, cv::Size(w, w), cv::Point(-1, -1), cv::BORDER_REFLECT);
    return outputImage;
}

cv::Mat guidedFilterGray(cv::Mat &inputImage, cv::Mat &guideImage, int radius, float epsilon)
{
    // split inputImage
    int inputChannels = inputImage.channels();
    int inputType = inputImage.type();
    cv::Mat inputImageSplit[inputChannels];
    cv::split(inputImage, inputImageSplit);

    // u8 to f32
    cv::Mat inputImage_f, guideImage_f;
    guideImage.convertTo(guideImage_f, CV_32FC1, 1/255.0);

    cv::Mat outputImageSplit[inputChannels];
    for(int chIdx = 0; chIdx < inputChannels; chIdx++)
    {
        inputImageSplit[chIdx].convertTo(inputImage_f, CV_32FC1, 1/255.0);

        // step 1
        int win = radius * 2 + 1;
        cv::Mat meanGuide = meanFilter(guideImage_f, win);
        cv::Mat meanInput = meanFilter(inputImage_f, win);
        cv::Mat corrGuide = meanFilter(guideImage_f.mul(guideImage_f), win);
        cv::Mat corrGuideInput = meanFilter(guideImage_f.mul(inputImage_f), win);
        // step 2
        cv::Mat varGuide = corrGuide - meanGuide.mul(meanGuide);
        cv::Mat covGuideInput = corrGuideInput - meanGuide.mul(meanInput);
        // step 3
        cv::Mat a = covGuideInput / (varGuide + epsilon);
        cv::Mat b = meanInput - a.mul(meanGuide);
        // step 4
        cv::Mat meanA = meanFilter(a, win);
        cv::Mat meanB = meanFilter(b, win);
        // step 5
        cv::Mat q = meanA.mul(guideImage_f) + meanB;
        outputImageSplit[chIdx] = q.clone();
    }

    cv::Mat outputImage_f, outputImage;
    cv::merge(outputImageSplit, inputChannels, outputImage_f);
    outputImage_f.convertTo(outputImage, inputType, 255.0);
    return outputImage;
}


cv::Mat guidedFilterColor(cv::Mat &inputImage, cv::Mat &guideImage, int radius, float epsilon)
{
    // split inputImage
    int inputChannels = inputImage.channels();
    int inputType = inputImage.type();
    cv::Mat inputImageSplit[inputChannels];
    cv::split(inputImage, inputImageSplit);

    // split guideImage
    int guideChannels = guideImage.channels();
    int guideType = guideImage.type();

    cv::Mat guideImage_f;
    guideImage.convertTo(guideImage_f, CV_32FC3, 1/255.0);
    cv::Mat guideImageSplit_f[guideChannels];
    cv::split(guideImage_f, guideImageSplit_f);

    cv::Mat guideImageB = guideImageSplit_f[0];
    cv::Mat guideImageG = guideImageSplit_f[1];
    cv::Mat guideImageR = guideImageSplit_f[2];

    // u8 to f32
    cv::Mat inputImage_f;
    cv::Mat outputImageSplit[inputChannels];
    for(int chIdx = 0; chIdx < inputChannels; chIdx++)
    {
        inputImageSplit[chIdx].convertTo(inputImage_f, CV_32FC1, 1/255.0);

        // step 1
        int win = radius * 2 + 1;
        cv::Mat meanGuideB = meanFilter(guideImageB, win);
        cv::Mat meanGuideG = meanFilter(guideImageG, win);
        cv::Mat meanGuideR = meanFilter(guideImageR, win);

        cv::Mat meanInput = meanFilter(inputImage_f, win);
        cv::Mat meanInputGuideB = meanFilter(inputImage_f.mul(guideImageB), win);
        cv::Mat meanInputGuideG = meanFilter(inputImage_f.mul(guideImageG), win);
        cv::Mat meanInputGuideR = meanFilter(inputImage_f.mul(guideImageR), win);

        // covariance matrix of color guide image
        //  BB   BG  BR
        // [GB]  GG  GR
        // [RB] [RG] RR

        cv::Mat varGuide_BB = meanFilter(guideImageB.mul(guideImageB), win) - meanGuideB.mul(meanGuideB) + epsilon;
        cv::Mat varGuide_BG = meanFilter(guideImageB.mul(guideImageG), win) - meanGuideB.mul(meanGuideG);
        cv::Mat varGuide_BR = meanFilter(guideImageB.mul(guideImageR), win) - meanGuideB.mul(meanGuideR);
        cv::Mat varGuide_GG = meanFilter(guideImageG.mul(guideImageG), win) - meanGuideG.mul(meanGuideG) + epsilon;
        cv::Mat varGuide_GR = meanFilter(guideImageG.mul(guideImageR), win) - meanGuideG.mul(meanGuideR);
        cv::Mat varGuide_RR = meanFilter(guideImageR.mul(guideImageR), win) - meanGuideR.mul(meanGuideR) + epsilon;

        // calculate inverse of cov matrix
        cv::Mat inv_BB = varGuide_GG.mul(varGuide_RR) - varGuide_GR.mul(varGuide_GR);
        cv::Mat inv_BG = varGuide_GR.mul(varGuide_BR) - varGuide_BG.mul(varGuide_RR);
        cv::Mat inv_BR = varGuide_BG.mul(varGuide_GR) - varGuide_GG.mul(varGuide_BR);
        cv::Mat inv_GG = varGuide_BB.mul(varGuide_RR) - varGuide_BR.mul(varGuide_BR);
        cv::Mat inv_GR = varGuide_BG.mul(varGuide_BR) - varGuide_BB.mul(varGuide_GR);
        cv::Mat inv_RR = varGuide_BB.mul(varGuide_GG) - varGuide_BG.mul(varGuide_BG);
        cv::Mat covDet = inv_BB.mul(varGuide_BB) + inv_BG.mul(varGuide_BG) + inv_BR.mul(varGuide_BR);
        
        inv_BB = inv_BB / covDet;
        inv_BG = inv_BG / covDet;
        inv_BR = inv_BR / covDet;
        inv_GG = inv_GG / covDet;
        inv_GR = inv_GR / covDet;
        inv_RR = inv_RR / covDet;

        cv::Mat covInputGuideB = meanInputGuideB - meanGuideB.mul(meanInput);
        cv::Mat covInputGuideG = meanInputGuideG - meanGuideG.mul(meanInput);
        cv::Mat covInputGuideR = meanInputGuideR - meanGuideR.mul(meanInput);

        cv::Mat aB = inv_BB.mul(covInputGuideB) + inv_BG.mul(covInputGuideG) + inv_BR.mul(covInputGuideR);
        cv::Mat aG = inv_BG.mul(covInputGuideB) + inv_GG.mul(covInputGuideG) + inv_GR.mul(covInputGuideR);
        cv::Mat aR = inv_BR.mul(covInputGuideB) + inv_GR.mul(covInputGuideG) + inv_RR.mul(covInputGuideR);

        cv::Mat b = meanInput - aB.mul(meanGuideB) - aG.mul(meanGuideG) - aR.mul(meanGuideR);

        outputImageSplit[chIdx] = meanFilter(aB, win).mul(guideImageB)
                                + meanFilter(aG, win).mul(guideImageG)
                                + meanFilter(aR, win).mul(guideImageR) + meanFilter(b, win);
    }

    cv::Mat outputImage_f, outputImage;
    cv::merge(outputImageSplit, inputChannels, outputImage_f);
    outputImage_f.convertTo(outputImage, inputType, 255.0);
    return outputImage;
}
