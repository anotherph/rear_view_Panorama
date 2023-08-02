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

Panorama::Panorama():sizeWin(200), ind_l(0), ind_r(0),num_k(15), i_width(1920), i_hight (1080)
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

Mat Panorama::Trasf(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode)
{
    Mat dst_;
    vector<Point2f> pts_;
    Mat H; 

    if (mode==0) //left
    {
        hconcat(Mat(dst.rows,dst.cols,CV_8UC3,Scalar(0,0,0)),dst,dst);
        ptsShift(pts_d,src.cols);ptsShift(pts_s,src.cols);

        H = findHomography(pts_d, pts_s, RANSAC); // calculate perspective transform matrix
        warpPerspective(dst,dst_,H, Size(dst.cols*1,dst.rows*1)); // apply perspective transform to image plane
        perspectiveTransform(pts_d,pts_,H);
        H_l=H;
        checkMapping(dst_,src,pts_,pts_s,mode);
    }

    if (mode==2) //right
    {
        hconcat(dst,Mat(dst.rows,dst.cols,CV_8UC3,Scalar(0,0,0)),dst);
        H = findHomography(pts_d, pts_s, RANSAC); // calculate perspective transform matrix
        warpPerspective(dst,dst_,H, Size(dst.cols*1,dst.rows*1)); // apply perspective transform to image plane
        perspectiveTransform(pts_d,pts_,H);
        H_r=H;
        checkMapping(dst_,src,pts_,pts_s,mode);
    }
        
    return dst_;
}

Mat Panorama::Blend(Mat imgL, Mat imgC, Mat imgR)
{
    hconcat(imgC,Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgC);
    hconcat(Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgC,imgC);
    hconcat(imgL,Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgL);
    hconcat(Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgR,imgR);

    int offset=sizeWin/2;
    int pos_l=i_width+offset;
    int pos_r = 2*i_width-offset; 
    int cnt = 0;
    Mat img_blend; // final panorama image 

    for (int cnt_x=0;cnt_x<imgL.rows;++cnt_x)
    {   
        cnt =0;
        for (int cnt_y=0;cnt_y<pos_l-offset;++cnt_y)
        {
            imgL.at<Vec3b>(cnt_x,cnt_y)=imgL.at<Vec3b>(cnt_x,cnt_y);
            imgC.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
        }

        for (int cnt_y=pos_l-offset;cnt_y<pos_l+offset;++cnt_y)
        {
            imgL.at<Vec3b>(cnt_x,cnt_y)=imgL.at<Vec3b>(cnt_x,cnt_y)*((2*offset-cnt)/double(2*offset));
            imgC.at<Vec3b>(cnt_x,cnt_y)=imgC.at<Vec3b>(cnt_x,cnt_y)*((cnt)/double(2*offset));
            cnt++;
        }

        for (int cnt_y=pos_l+offset;cnt_y<imgL.cols;++cnt_y)
        {
            imgL.at<Vec3b>(cnt_x,cnt_y)=Vec3b(0,0,0);
            imgC.at<Vec3b>(cnt_x,cnt_y)=imgC.at<Vec3b>(cnt_x,cnt_y);
        }
    }
    // imwrite(".././Img/panorama_d/pts/L.jpg", imgL);
    // imwrite(".././Img/panorama_d/pts/LC.jpg", imgC);

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
            if (cnt_y==pos_r)
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

    // imwrite(".././Img/panorama_d/pts/R.jpg", imgR);
    // imwrite(".././Img/panorama_d/pts/RC.jpg", imgC);

    img_blend=imgL+imgC+imgR;
    return img_blend;
}

void Panorama::checkMapping(Mat dst, Mat src, vector<Point2f> pts_d, vector<Point2f> pts_s, int mode)
{
    Point pts1_v, pts2_v;
    int num_k=pts_d.size();
    Mat img_check;
    if (mode==0)
    {
        img_check=src;
        hconcat(dst,src,img_check);
        for (size_t i=0; i<num_k; i++) {
            pts1_v.x = pts_d[i].x; pts1_v.y = pts_d[i].y;
            pts2_v.x = pts_s[i].x+src.cols; pts2_v.y = pts_s[i].y;
            string s1 = to_string(i);
            circle(img_check,pts1_v,1, Scalar(255,0,255),10,-1,0);putText(img_check,s1,pts1_v,1,3,Scalar::all(0),2); // magenta
            circle(img_check,pts2_v,1, Scalar(0,255,255),10,-1,0);putText(img_check,s1,pts2_v,1,3,Scalar::all(0),2); //yellow
            line(img_check,pts1_v,pts2_v,Scalar::all(255), 1, 8, 0);
        }
        imwrite(".././Img/panorama_d/result/feature_l.jpg", img_check);
    }
        
    if (mode==2)
    {
         hconcat(src,dst,img_check);
        for (size_t i=0; i<num_k; i++) {
            pts1_v.x = pts_d[i].x+src.cols; pts1_v.y = pts_d[i].y;
            pts2_v.x = pts_s[i].x; pts2_v.y = pts_s[i].y;
            string s1 = to_string(i);
            circle(img_check,pts1_v,1, Scalar(255,0,255),10,-1,0);putText(img_check,s1,pts1_v,1,3,Scalar::all(0),2);
            circle(img_check,pts2_v,1, Scalar(0,255,255),10,-1,0);putText(img_check,s1,pts2_v,1,3,Scalar::all(0),2);
            line(img_check,pts1_v,pts2_v,Scalar::all(255), 1, 8, 0);
        }
        imwrite(".././Img/panorama_d/result/feature_r.jpg", img_check);
    }
       
}

void Panorama::ptsShift(vector<Point2f>& pts_, int val_shift)
{
    for (int cnt_=0;cnt_<pts_.size();++cnt_)
    {
        pts_[cnt_].x+=val_shift;
    }
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

