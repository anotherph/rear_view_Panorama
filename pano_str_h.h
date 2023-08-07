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
    int                                     num_k;                  // number of keypoints
    int                                     i_width;                // image width of each webcam
    int                                     i_hight;                // image height of each webcam
    Mat                                     H_l;                    // perspective transform matrix for left image
    Mat                                     H_r;                    // perspective transform matrix for right image
    vector<Point2f>                         m_pts_left;             // load the keypoints from "dir_pts_left"
    vector<Point2f>                         m_pts_center_l;         // load the keypoints from "dir_pts_center_l"
    vector<Point2f>                         m_pts_right;            // load the keypoints from "dir_pts_right"
    vector<Point2f>                         m_pts_center_r;         // load the keypoints from "dir_pts_right_l"
    string                                  dir_pts_left;
    string                                  dir_pts_center_l;
    string                                  dir_pts_right;
    string                                  dir_pts_center_r;
    Mat                                     img_pano_t;
    Mat                                     img_p;
    Mat                                     dst_l;
    Mat                                     dst_r;
    Mat                                     mask_l;
    Mat                                     mask_cl;
    Mat                                     mask_r;
    Mat                                     mask_cr;
    
public:
    Panorama();
    ~Panorama();
    virtual vector<Point2f> readTxtFile(string path_str); 
    virtual void Trasf_rt(vector<Point2f> pts_d, vector<Point2f> pts_s, int mode);
    // virtual Mat Trasf(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode); // dst must be center image
    virtual Mat Blend_tr(Mat& imgL, Mat& imgC, Mat& imgR);
    // virtual Mat Blend(Mat imgL, Mat imgC, Mat imgR);
    // virtual void checkMapping(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode);
    virtual void ptsShift(vector<Point2f>& pts_, int val_shift);
    // virtual Mat makePano2D(Mat& img_srcL, Mat& img_srcC, Mat& img_srcR);
    virtual void setMask(void);
};