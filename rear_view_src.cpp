#define _DLL_EXPORTS
#include "./rear_view_h.h"
#include <opencv2/imgproc/types_c.h>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#define left 0
#define right 1

using namespace cv;
using namespace std;

Panorama::Panorama():sizeWin(800)
{
 // constructor 
}

Panorama::~Panorama()
{
    //destructor
}

Mat Panorama::Trasf(Mat dst, Mat src, int mode)
{
    return 0;
}

Mat Panorama::Blend(Mat imgL, Mat imgC, Mat imgR)
{
    return 0; 
}

