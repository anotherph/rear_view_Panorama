// SIFT & RANSAC example for image stitching

#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp> 
#include <opencv2/stitching.hpp>
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
        vecTemp.push_back(cv::Point2f(x, y));
    }
}

int main ()
{
    // load images and make images being in gray scale

    Mat img_src1, img_src2; 

    img_src1 = imread(".././Img/IMG_8629.JPEG"); 
    img_src2 = imread(".././Img/IMG_8630.JPEG");  

    // img_src1 = imread(".././Img/left.jpg"); 
    // img_src2 = imread(".././Img/right.jpg");  

    Mat img_check; // check the features 
    hconcat(img_src1,img_src2,img_check);

    cvtColor(img_src1,img_src1,COLOR_RGB2GRAY);
    cvtColor(img_src2,img_src2,COLOR_RGB2GRAY);

    // SIFT 

    Ptr<Feature2D> detector = SIFT::create();

    vector<KeyPoint> kpts1, kpts2;
    Mat desc1, desc2; 

    detector->detectAndCompute(img_src1, Mat(), kpts1, desc1);
    detector->detectAndCompute(img_src2, Mat(), kpts2, desc2);

    // matching detected features
    desc1.convertTo(desc1, CV_32F);
    desc2.convertTo(desc2, CV_32F);

    Ptr<DescriptorMatcher> matcher = FlannBasedMatcher::create();
    vector<DMatch> matches;
    matcher->match(desc1, desc2, matches);

    int num_k=20; // number of kpts for matching
    sort(matches.begin(), matches.end());
    vector<DMatch> good_matches(matches.begin(), matches.begin() + num_k); // find best match from 1st place to num_k-th place in order 

    vector<Point2f> pts1, pts2;
    for (size_t i = 0; i < good_matches.size(); i++) {
        pts1.push_back(kpts1[good_matches[i].queryIdx].pt);
        pts2.push_back(kpts2[good_matches[i].trainIdx].pt);
    }

    // // // use pts1 which are selected manually
    // vector<Point2f> pts1, pts2;
  
    // imshow("image", img_src1);
    // cv::waitKey(1);
    // while(1)
    // {
    //     int key = cv::waitKey(10);
    //     setMouseCallback("image", OnMouseAction);
    //     if (key == 'q')
    //         break;
    // }
    // pts1 = vecTemp;
    // vecTemp.clear();
    // cv::destroyWindow("image");

    // imshow("image", img_src2);
    // cv::waitKey(1);
    // while(1)
    // {
    //     int key = cv::waitKey(10);
    //     setMouseCallback("image", OnMouseAction);
    //     if (key == 'q')
    //         break;
    // }
    // pts2 = vecTemp;
    // vecTemp.clear();
    // cv::destroyWindow("image");

    // Homograpy 

    Mat H = findHomography(pts2, pts1, RANSAC); // calculate perspective transform matrix
    Mat img_dst1, img_dst2, img_dst;
    img_src1.copyTo(img_dst1);
    warpPerspective(img_src2,img_dst2, H, Size(img_dst2.cols,img_dst2.rows)); // apply perspective transform to image plane
    hconcat(img_dst1,img_dst2,img_dst); 
    int size_x=img_dst.rows;
    int size_y=img_dst.cols;

    // check the Perspective transformed pts
    vector<Point2f> pts2_tr;
    perspectiveTransform(pts2,pts2_tr,H); // apply perspective transform to points
        
    // Stitching 

    Mat temp1, temp2, img_f;
    temp1 = img_dst1(Rect(0,0,img_dst1.cols-pts1[10].y,img_dst1.rows));
    temp2 = img_dst2(Rect(pts2_tr[10].y,0,img_dst2.cols-pts2_tr[10].y,img_dst2.rows));
    hconcat(temp1,temp2,img_f);
    imwrite(".././Img/sift_ransac.jpg", img_f);
    // edit the index of feature points

    // show the feature points in two adject images
    

    for (size_t i=0; i<num_k; i++) {
        circle(img_check,Point(pts1[i].x,pts1[i].y),1, Scalar(255,0,255),10,-1,0);
        circle(img_check,Point(img_src1.cols+pts2[i].x,pts2[i].y),1, Scalar(0,255,255),10,-1,0);
        line(img_check,Point(pts1[i].x,pts1[i].y),Point(img_src1.cols+pts2[i].x,pts2[i].y),Scalar::all(255), 1, 8, 0);
    }

    imwrite(".././Img/feature.jpg", img_check);

    for (size_t i=0; i<num_k; i++) {
        circle(img_dst,Point(pts1[i].x,pts1[i].y),1, Scalar(255,0,255),10,-1,0);
        circle(img_dst,Point(img_src1.cols+pts2_tr[i].x,pts2_tr[i].y),1, Scalar(0,255,255),10,-1,0);
        line(img_dst,Point(pts1[i].x,pts1[i].y),Point(img_src1.cols+pts2_tr[i].x,pts2_tr[i].y),Scalar::all(255), 1, 8, 0);
    }

    imwrite(".././Img/feature_in_perspective.jpg", img_dst);
    return 0;

}