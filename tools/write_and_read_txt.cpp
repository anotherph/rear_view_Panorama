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
    }
}

// int main(int argc, char* argv[]) // use parser when you make the txt file. 
int main ()
{
    // parser
    String name = "center";
    int num_k=15;

    // default path; if you want to edit the absolute path of src, plz edit here and complie& build again. 
    String path = "/home/jekim/workspace/rear_view_Panorama/Img/panorama_d";
    String path_file = "/pts/"+name+"_r.txt";
    String path_img = "/img_original/"+name+".jpg";
    String path_imgwpts = "/pts/"+name+"_r_pts.jpg";

    // load the image 
    Mat img_src = imread(path+path_img);
    Mat img_pts; img_src.copyTo(img_pts);
    ofstream txtFile(path+path_file);
    vector<Point2f> pts;

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
    pts = vecTemp;
    vecTemp.clear();
    cv::destroyWindow("image");

    // save the keypoints in text file
    for (int i=0;i<num_k;i++)
        txtFile << pts[i] << "\n"; 
    txtFile.close();

    // save the images with keypoints
     for (size_t i=0; i<num_k; i++) {
        string s1 = to_string(i);
        circle(img_pts,pts[i],1, Scalar(255,0,255),10,-1,0);putText(img_pts,s1,pts[i],1,3,Scalar::all(0),2);
    }

    imwrite(path+path_imgwpts,img_pts);

    return 0;

}
