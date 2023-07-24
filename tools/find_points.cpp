#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
using std::experimental::filesystem::directory_iterator;
namespace fs = std::experimental::filesystem;

vector<Point2f> vecTemp;
void OnMouseAction(int event, int x, int y, int flags, void *para)
{
    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        vecTemp.push_back(cv::Point2f(x, y));
        cout<<cv::Point2f(x, y)<<endl;
    }
}

// int main(int argc, char* argv[]) // use parser when you make the txt file. 
int main ()
{
    // default path; if you want to edit the absolute path of src, plz edit here and complie& build again. 
    String path = ".././Img/panorama_d/pts/pano.jpg";

    // load the image 
    Mat img_src = imread(path);

    // select the keypoints manually
    imshow("image", img_src);
    cv::waitKey(1);
    while(1)
    {
        int key = cv::waitKey(10);
        setMouseCallback("image", OnMouseAction);
        if (key == 'q')
            break;
    }
    // pts = vecTemp;
    vecTemp.clear();
    cv::destroyWindow("image");

    return 0;

}
