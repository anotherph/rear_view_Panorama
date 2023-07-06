// SIFT & RANSAC example for image stitching

#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> 
#include <cmath>

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
        vecTemp.push_back(cv::Point2f(y, x));
    }
}

int main ()
{
    // load images and make images being in gray scale

    Mat left, right, center; 

    left = imread(".././Img/left.jpg"); 
    right = imread(".././Img/right.jpg"); 
    center = imread(".././Img/right.jpg"); 

    Mat check_; // check the features 
    hconcat(left,center,check_);

    cvtColor(left,left,COLOR_RGB2GRAY);
    cvtColor(right,right,COLOR_RGB2GRAY);
    cvtColor(center,center,COLOR_RGB2GRAY);

    // // SIFT 

    // Ptr<Feature2D> detector = SIFT::create();

    // vector<KeyPoint> kpts1, kpts2;
    // Mat desc1, desc2; 

    // detector->detectAndCompute(left, Mat(), kpts1, desc1);
    // detector->detectAndCompute(center, Mat(), kpts2, desc2);

    // // matching detected features
    // desc1.convertTo(desc1, CV_32F);
    // desc2.convertTo(desc2, CV_32F);

    // Ptr<DescriptorMatcher> matcher = FlannBasedMatcher::create();
    // vector<DMatch> matches;
    // matcher->match(desc1, desc2, matches);

    // int num_k=100; // number of kpts for matching
    // sort(matches.begin(), matches.end());
    // vector<DMatch> good_matches(matches.begin(), matches.begin() + num_k); // find best match from 1st place to 100th place in order 

    // vector<Point2f> pts1, pts2;
    // for (size_t i = 0; i < good_matches.size(); i++) {
    //     pts1.push_back(kpts1[good_matches[i].queryIdx].pt);
    //     pts2.push_back(kpts2[good_matches[i].trainIdx].pt);
    // }
    // use pts1 which are selected manually
    vector<Point2f> pts1, pts2;
    // cornerSubPix(left, pts1, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 100, 0.001));
    // cornerSubPix(center, pts2, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 100, 0.001));

    imshow("image", left);
    cv::waitKey(1);
    while(1)
    {
        int key = cv::waitKey(10);
        setMouseCallback("image", OnMouseAction);
        if (key == 'q')
            break;
    }
    pts1 = vecTemp;
    vecTemp.clear();
    cv::destroyWindow("image");

    imshow("image", right);
    cv::waitKey(1);
    while(1)
    {
        int key = cv::waitKey(10);
        setMouseCallback("image", OnMouseAction);
        if (key == 'q')
            break;
    }
    pts2 = vecTemp;
    vecTemp.clear();
    cv::destroyWindow("image");

    // Homograpy 

    Mat H = findHomography(pts2, pts1, RANSAC);
    imwrite(".././Img/sift_ransac.jpg", H);

    // show the feature points in two adject images

    for (size_t i=0; i<16; i++) {
        circle(check_,Point(pts1[i].y,pts1[i].x),1, Scalar(255,0,255),10,-1,0);
        circle(check_,Point(left.cols+pts2[i].y,pts2[i].x),1, Scalar(0,255,255),10,-1,0);
        line(check_,Point(pts1[i].y,pts1[i].x),Point(left.cols+pts2[i].y,pts2[i].x),Scalar::all(255), 1, 8, 0);
    }
    
    imwrite(".././Img/feature.jpg", check_);

    int a=1;

    return 0;
















    //  
}