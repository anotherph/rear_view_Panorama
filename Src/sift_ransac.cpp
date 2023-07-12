// SIFT & RANSAC example for image stitching
// left & right -- with two images

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

void blending_img(int win_size, Mat& src1, Mat& src2, Mat& dst)
{
    int pos=src1.cols/2;
    int offset=win_size/2;
    int bar=pos-offset; 
    int cnt = 0;
    Mat temp; // for crop the black area..

    for (int cnt_x=0;cnt_x<src1.rows;++cnt_x)
    {   
        cnt =0;
        for (int cnt_y=0;cnt_y<bar-offset;++cnt_y)
        {
            src1.at<Vec3b>(cnt_x,cnt_y)=src1.at<Vec3b>(cnt_x,cnt_y);
            src2.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
        }

        for (int cnt_y=bar-offset;cnt_y<bar+offset;++cnt_y)
        {
            src1.at<Vec3b>(cnt_x,cnt_y)=src1.at<Vec3b>(cnt_x,cnt_y)*((2*offset-cnt)/double(2*offset));
            src2.at<Vec3b>(cnt_x,cnt_y)=src2.at<Vec3b>(cnt_x,cnt_y)*((cnt)/double(2*offset));
            cnt++;
        }

        for (int cnt_y=bar+offset;cnt_y<src1.cols;++cnt_y)
        {
            src1.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
            src2.at<Vec3b>(cnt_x,cnt_y)=src2.at<Vec3b>(cnt_x,cnt_y);
        }
    }
    dst=src1+src2;
}

int main ()
{
    // load images and make images being in gray scale

    Mat img_src1, img_src2, img_src1_, img_src2_; 

    // img_src1 = imread(".././Img/IMG_8629.JPEG"); 
    // img_src2 = imread(".././Img/IMG_8630.JPEG");  

    img_src1 = imread(".././Img/center.jpg"); 
    img_src2 = imread(".././Img/right.jpg");  

    Mat img_check, img_check_; // check the features 
    hconcat(img_src1,img_src2,img_check);

    cvtColor(img_src1,img_src1_,COLOR_RGB2GRAY);
    cvtColor(img_src2,img_src2_,COLOR_RGB2GRAY);

    // // SIFT 

    // Ptr<Feature2D> detector = SIFT::create();

    // vector<KeyPoint> kpts1, kpts2;
    // Mat desc1, desc2; 

    // detector->detectAndCompute(img_src1_, Mat(), kpts1, desc1);
    // detector->detectAndCompute(img_src2_, Mat(), kpts2, desc2);

    // // matching detected features
    // desc1.convertTo(desc1, CV_32F);
    // desc2.convertTo(desc2, CV_32F);

    // Ptr<DescriptorMatcher> matcher = FlannBasedMatcher::create();
    // vector<DMatch> matches;
    // matcher->match(desc1, desc2, matches);

    // int num_k=20; // number of kpts for matching
    // sort(matches.begin(), matches.end());
    // vector<DMatch> good_matches(matches.begin(), matches.begin() + num_k); // find best match from 1st place to num_k-th place in order 

    // vector<Point2f> pts1, pts2;
    // for (size_t i = 0; i < good_matches.size(); i++) {
    //     pts1.push_back(kpts1[good_matches[i].queryIdx].pt);
    //     pts2.push_back(kpts2[good_matches[i].trainIdx].pt);
    // }

    // // use pts1 which are selected manually
    // vector<Point2f> pts1, pts2;
    // int num_k=16; // number of kpts for matching
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

    // img_dst=Mat(img_src1.rows,img_src1.cols*2,CV_8UC3,Scalar(0,0,0));
    img_src1.copyTo(img_dst1);
    warpPerspective(img_src2,img_dst2, H, Size(img_src2.cols*2,img_src2.rows)); // apply perspective transform to image plane
    hconcat(img_src1,img_dst2(Rect(0,0,img_src2.cols,img_src2.rows)),img_check_); 
    hconcat(img_dst1,Mat(img_src1.rows,img_src1.cols,CV_8UC3,Scalar(0,0,0)),img_dst1);

    // check the Perspective transformed pts
    vector<Point2f> pts2_tr;
    perspectiveTransform(pts2,pts2_tr,H); // apply perspective transform to points
        
    // Stitching (masking & blending)
    int sizeWin=800;
    img_dst = Mat(img_dst1.rows,img_dst1.cols*2,img_dst1.type());
    blending_img(sizeWin,img_dst1,img_dst2,img_dst);
    imwrite(".././Img/img_left.jpg", img_dst1);
    imwrite(".././Img/img_right.jpg", img_dst2);
    imwrite(".././Img/img_blending.jpg", img_dst);

    // show the feature points in two adject images
    
    Point pts1_v, pts2_v;
    for (size_t i=0; i<num_k; i++) {
        pts1_v.x = pts1[i].x; pts1_v.y = pts1[i].y;
        pts2_v.x = img_src1.cols+pts2[i].x; pts2_v.y = pts2[i].y;
        string s1 = to_string(i);
        circle(img_check,pts1_v,1, Scalar(255,0,255),10,-1,0);putText(img_check,s1,pts1_v,1,3,Scalar::all(0),2);
        circle(img_check,pts2_v,1, Scalar(0,255,255),10,-1,0);putText(img_check,s1,pts2_v,1,3,Scalar::all(0),2);
        line(img_check,pts1_v,pts2_v,Scalar::all(255), 1, 8, 0);
    }

    imwrite(".././Img/feature.jpg", img_check);

    for (size_t i=0; i<num_k; i++) {
        pts1_v.x = pts1[i].x; pts1_v.y = pts1[i].y;
        pts2_v.x = img_src1.cols+pts2_tr[i].x; pts2_v.y = pts2_tr[i].y;
        string s1 = to_string(i);
        circle(img_check_,pts1_v,1, Scalar(255,0,255),10,-1,0);putText(img_check_,s1,pts1_v,1,3,Scalar::all(0),2);
        circle(img_check_,pts2_v,1, Scalar(0,255,255),10,-1,0);putText(img_check_,s1,pts2_v,1,3,Scalar::all(0),2);
        line(img_check_,pts1_v,pts2_v,Scalar::all(255), 1, 8, 0);
    }

    imwrite(".././Img/feature_in_perspective.jpg", img_check_);
    return 0;

}