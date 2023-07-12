#pragma once
#include <opencv2/opencv.hpp>
#include <experimental/filesystem>
#include <fstream>

using namespace cv;
using namespace std;

class Panorama
{
private:
    int                                     sizeWin;                // blending window

public:
    Panorama();
    ~Panorama();
    virtual Mat Trasf(Mat dst, Mat src, int mode); // dst must be center image
    virtual Mat Blend(Mat imgL, Mat imgC, Mat imgR);
};