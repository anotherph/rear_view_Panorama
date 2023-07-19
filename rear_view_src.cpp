#define _DLL_EXPORTS
#include "./rear_view_h.h"
#include <opencv2/imgproc/types_c.h>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#define left 0
#define center 1
#define right 2

using namespace cv;
using namespace std;

Panorama::Panorama():sizeWin(400), ind_l(0), ind_r(0)
{
 // constructor 
}

Panorama::~Panorama()
{
    //destructor
}

vector<Point2f> Panorama::readTxtFile(string path_str)
{
    vector<Point2f> pts;
    Point pts_;
    string hzText;
    int num_k = 8;
    string::size_type n; 

    std::ifstream readFile;            
    readFile.open(path_str);    
    if(readFile.is_open())   
        {
        // save the keypoints in text file
            for (int i=0;i<num_k;i++)
            {
                getline(readFile,hzText);
                n = hzText.find(",");
                pts_.x = stoi(hzText.substr(1,n));
                pts_.y = stoi(hzText.substr(n+1,hzText.size()-1));
                pts.push_back(pts_);
            } 
        }
    readFile.close();   

    return pts; 
}

// void Panorama::Trasf(Mat& dst, Mat& src, vector<Point2f>& pts_d, vector<Point2f>& pts_s, int mode)
Mat Panorama::Trasf(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode)
{
    // 1- center(dst), 2-right(src) findHomography(2,1)
    // panorama->Trasf(img_srcL,img_srcC,m_pts_left,m_pts_center_l,left);

    Mat H = findHomography(pts_d, pts_s, RANSAC); // calculate perspective transform matrix
    Mat dst_;
    vector<Point2f> pts_; 
    warpPerspective(dst,dst_, H, Size(dst.cols*2,dst.rows*1)); // apply perspective transform to image plane
    perspectiveTransform(pts_d,pts_,H);

    if (mode==0) //left
        findMax(pts_);
        m_blend_cl=vector<Point2f>(1,Point2f(pts_s[ind_l].x,pts_s[ind_l].y));
    if (mode==2) //right
        findMin(pts_);
        m_blend_cr=vector<Point2f>(1,Point2f(pts_s[ind_r].x,pts_s[ind_r].y));

    return dst_;

}

// void Panorama::Blend(Mat imgL, Mat imgC, Mat imgR)
Mat Panorama::Blend(Mat imgL, Mat imgC, Mat imgR)
{
    hconcat(imgC,Mat(imgC.rows,imgC.cols,CV_8UC3,Scalar(0,0,0)),imgC);

    int offset=sizeWin/2;
    int pos_l=m_blend_l[0].x;
    int bar_l=pos_l-sizeWin;
    // int pos_r=m_blend_r[0].x;
    int pos_r = 1500; // select the blending point
    int bar_r=pos_r-sizeWin; 
    int cnt = 0;
    Mat img_blend; // final panorama image 

    for (int cnt_x=0;cnt_x<imgL.rows;++cnt_x)
    {   
        cnt =0;
        for (int cnt_y=0;cnt_y<bar_l-offset;++cnt_y)
        {
            imgL.at<Vec3b>(cnt_x,cnt_y)=imgL.at<Vec3b>(cnt_x,cnt_y);
            imgC.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
        }

        for (int cnt_y=bar_l-offset;cnt_y<bar_l+offset;++cnt_y)
        {
            imgL.at<Vec3b>(cnt_x,cnt_y)=imgL.at<Vec3b>(cnt_x,cnt_y)*((2*offset-cnt)/double(2*offset));
            imgC.at<Vec3b>(cnt_x,cnt_y)=imgC.at<Vec3b>(cnt_x,cnt_y)*((cnt)/double(2*offset));
            cnt++;
        }

        for (int cnt_y=bar_l+offset;cnt_y<imgL.cols;++cnt_y)
        {
            imgL.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
            imgC.at<Vec3b>(cnt_x,cnt_y)=imgC.at<Vec3b>(cnt_x,cnt_y);
        }
    }

    for (int cnt_x=0;cnt_x<imgR.rows;++cnt_x)
    {   
        cnt =0;
        for (int cnt_y=0;cnt_y<pos_r-offset;++cnt_y)
        {
            imgC.at<Vec3b>(cnt_x,cnt_y)=imgC.at<Vec3b>(cnt_x,cnt_y);
            imgR.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
        }

        for (int cnt_y=pos_r-offset;cnt_y<pos_r+offset;++cnt_y)
        {
            imgC.at<Vec3b>(cnt_x,cnt_y)=imgC.at<Vec3b>(cnt_x,cnt_y)*((2*offset-cnt)/double(2*offset));
            imgR.at<Vec3b>(cnt_x,cnt_y)=imgR.at<Vec3b>(cnt_x,cnt_y)*((cnt)/double(2*offset));
            cnt++;
        }

        for (int cnt_y=pos_r+offset;cnt_y<imgR.cols;++cnt_y)
        {
            imgC.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
            imgR.at<Vec3b>(cnt_x,cnt_y)=imgR.at<Vec3b>(cnt_x,cnt_y);
        }
    }

    img_blend=imgL+imgC+imgR;
    return img_blend;
}

void Panorama::findMax(vector<Point2f>& pts_)
{   
    ind_l = 0;
    m_blend_l=vector<Point2f>(1,Point2f(pts_[0].x,pts_[0].y));
    for (int i=0;i<pts_.size();++i)
    {
        if (pts_[i].x>m_blend_l[0].x)
            m_blend_l[0]=pts_[i];ind_l = i; 
    }
}

void Panorama::findMin(vector<Point2f>& pts_)
{
    ind_r = 0; 
    m_blend_r=vector<Point2f>(1,Point2f(pts_[0].x,pts_[0].y));
    for (int i=0;i<pts_.size();++i)
    {
        if (pts_[i].x<m_blend_r[0].x)
            m_blend_r[0]=pts_[i];ind_r = i; 
    }
}

