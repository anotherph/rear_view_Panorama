#include <opencv2/opencv.hpp>
#include "./pano_str_h.h"
#include <fstream>

#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <iomanip>
#include <cstring>

#define left 0
#define center 1
#define right 2

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    // ############ capture the video streaming ############
    int count = 0;
    Mat vi_left, vi_right, vi_center, vi_pano2D;

    int width = 1920;
	int height = 1080;

    // center_cam
	VideoCapture cap_center(4);
	cap_center.set(CAP_PROP_FRAME_WIDTH, width);
	cap_center.set(CAP_PROP_FRAME_HEIGHT, height);

	if (!cap_center.isOpened())
    {
        printf("Can't open the camera");
        return -1;
    }

	// left_cam
	VideoCapture cap_left(16);
	cap_left.set(CAP_PROP_FRAME_WIDTH, width);
	cap_left.set(CAP_PROP_FRAME_HEIGHT, height);

	if (!cap_left.isOpened())
    {
        printf("Can't open the camera");
        return -1;
    }

	// right_cam
	VideoCapture cap_right(10);
	cap_right.set(CAP_PROP_FRAME_WIDTH, width);
	cap_right.set(CAP_PROP_FRAME_HEIGHT, height);

	if (!cap_right.isOpened())
    {
        printf("Can't open the camera");
        return -1;
    }

    Panorama *panorama = new Panorama(); 

    while (1)
	{
        clock_t st, fi; 
        double duration;

        cap_center >> vi_center;
        cap_right >> vi_right;
        cap_left >> vi_left;

        st=clock();
        vi_pano2D=panorama->Blend_tr(vi_left,vi_center,vi_right);
        fi=clock();
        imshow("panorama image 2D", vi_pano2D);
        duration = (double)(fi - st) / CLOCKS_PER_SEC;
        cout << duration << "sec" << endl;

        if (waitKey(1) == 27)
            break;  // to turn off the window and break the while moon

	}
    
    return 0;

}