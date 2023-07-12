#include <opencv2/opencv.hpp>
#include "./rear_view_h.h"
#include <fstream>
#define left 0
#define right 1

using namespace cv;
using namespace std;

int main()
{
    // load images

    Mat img_srcL, img_srcR, img_srcC; // original images

    img_srcL = imread(".././Img/left.jpg"); 
    img_srcR = imread(".././Img/right.jpg");
    img_srcC = imread(".././Img/center.jpg");  

    Panorama *panorama = new Panorama();
    Mat img_dstL = panorama->Trasf(img_srcC,img_srcL,left);
    Mat img_dstR = panorama->Trasf(img_srcC,img_srcR,right);
    Mat img_blending = panorama->Blend(img_srcL,img_srcC,img_srcR);

    imwrite(".././Img/temp4/panorama.jpg", img_blending);
    
    return 0;

}