#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> 

using namespace cv;
using namespace std;
using std::experimental::filesystem::directory_iterator;
namespace fs = std::experimental::filesystem;

int main ()
{

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


	
    Mat img_center, img_right, img_left, img_temp, img_total;

	int count = 0;
	int num_count = 150;


	string path_img_t, path_img_l, path_img_r, path_img_c; 
	string path = "../Img";

	while (1)
	{
		path_img_t="../../Img/total.jpg";
		path_img_l="../../Img/left.jpg";
		path_img_r="../../Img/right.jpg";
		path_img_c="../../Img/center.jpg";
		count++;
		cap_center >> img_center;
		cap_right >> img_right;
		cap_left >> img_left;

		hconcat(img_left, img_center, img_temp);
		hconcat(img_temp, img_right, img_total);

		imshow("camera img", img_total);

		if (count==num_count)
		{
			imwrite(path_img_t, img_total); // save the calibration image
			imwrite(path_img_l, img_left); // save the calibration image
			imwrite(path_img_r, img_right); // save the calibration image
			imwrite(path_img_c, img_center); // save the calibration image

			cout<<" image capture complete! "<<endl;
			cout<<" press ESC to turn off the image window "<<endl;
		}
		
		if (waitKey(1) == 27)
			break;  // to turn off the window and break the while moon
			
	}

	return 0;
}
