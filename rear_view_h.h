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
    int                                     ind_l;                  // 
    int                                     ind_r; 
    vector<Point2f>                         m_blend_l;
    vector<Point2f>                         m_blend_r;
    vector<Point2f>                         m_blend_cl;
    vector<Point2f>                         m_blend_cr;
    int                                     i_width;
    int                                     i_hight;              
    
public:
    Panorama();
    ~Panorama();
    virtual vector<Point2f> readTxtFile(string path_str); 
    virtual Mat Trasf(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode); // dst must be center image
    // virtual void Trasf(Mat& dst, Mat& src, vector<Point2f>& pts_d, vector<Point2f>& pts_s, int mode); // dst must be center image
    virtual Mat Blend(Mat imgL, Mat imgC, Mat imgR);
    // virtual void Blend(Mat imgL, Mat imgC, Mat imgR);
    virtual void findMax(vector<Point2f>& pts_);
    virtual void findMin(vector<Point2f>& pts_);    
};