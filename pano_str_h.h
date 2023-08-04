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
    int                                     ind_l;                  
    int                                     ind_r; 
    int                                     num_k;
    int                                     i_width;
    int                                     i_hight;
    Mat                                     H_l;
    Mat                                     H_r;
    Mat                                     img_pano_t;
    string                                  dir_pts_left;
    string                                  dir_pts_center_l;
    string                                  dir_pts_right;
    string                                  dir_pts_center_r;
    
public:
    Panorama();
    ~Panorama();
    virtual vector<Point2f> readTxtFile(string path_str); 
    virtual Mat Trasf(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode); // dst must be center image
    virtual Mat Blend(Mat imgL, Mat imgC, Mat imgR);
    virtual void checkMapping(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode);
    virtual void ptsShift(vector<Point2f>& pts_, int val_shift);
    virtual Mat makePano2D(Mat& img_srcL, Mat& img_srcC, Mat& img_srcR);
};