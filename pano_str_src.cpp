#define _DLL_EXPORTS
#include "./pano_str_h.h"
#include <opencv2/imgproc/types_c.h>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <ctime>

#define left 0
#define center 1
#define right 2
using namespace cv;
using namespace std;

Panorama::Panorama():sizeWin(200), ind_l(0), ind_r(0),num_k(15), i_width(1920), i_hight (1080),
 dir_pts_left(".././Img/panorama_d/pts/left.txt"),
 dir_pts_center_l(".././Img/panorama_d/pts/center_l.txt"),
 dir_pts_right(".././Img/panorama_d/pts/right.txt"),
 dir_pts_center_r(".././Img/panorama_d/pts/center_r.txt"),
 mask_l(Mat(i_hight,3*i_width,CV_32FC3)),
 mask_cl(Mat(i_hight,3*i_width,CV_32FC3)),
 mask_r(Mat(i_hight,3*i_width,CV_32FC3)),
 mask_cr(Mat(i_hight,3*i_width,CV_32FC3))
{
    // constructor 
    // load keypoints and calculate the homography matrix
    m_pts_left=readTxtFile(dir_pts_left);
    m_pts_center_l=readTxtFile(dir_pts_center_l);
    m_pts_right=readTxtFile(dir_pts_right);
    m_pts_center_r=readTxtFile(dir_pts_center_r);
    Trasf_rt(m_pts_left,m_pts_center_l,left);
    Trasf_rt(m_pts_right,m_pts_center_r,right);
    setMask();
}

Panorama::~Panorama()
{
    //destructor
}

void Panorama::setMask(void)
{
    // calculate masks
    int offset=sizeWin/2;
    int pos_l=i_width+offset;
    int pos_r = 2*i_width-offset; 
    int cnt = 0;
    Mat d; 

    for (int cnt_x=0;cnt_x<i_hight;++cnt_x)
    {   
        cnt =0;
        for (int cnt_y=0;cnt_y<pos_l-offset;++cnt_y)
        {
            mask_l.at<Vec3f>(cnt_x,cnt_y)=Vec3f(1,1,1);
            mask_cl.at<Vec3f>(cnt_x,cnt_y)=Vec3f(0,0,0);
        }

        for (int cnt_y=pos_l-offset;cnt_y<pos_l+offset;++cnt_y)
        {
            float a1=(cnt)/float(2*offset);
            float b1=(2*offset-cnt)/float(2*offset);
            // cout<<mask_l.at<Vec3f>(cnt_x,cnt_y)<<endl;
            mask_l.at<Vec3f>(cnt_x,cnt_y)=Vec3f(b1,b1,b1);
            mask_cl.at<Vec3f>(cnt_x,cnt_y)=Vec3f(a1,a1,a1);
            cnt++;
        }

        for (int cnt_y=pos_l+offset;cnt_y<3*i_width;++cnt_y)
        {
            mask_l.at<Vec3f>(cnt_x,cnt_y)=Vec3f(0,0,0);
            mask_cl.at<Vec3f>(cnt_x,cnt_y)=Vec3f(1,1,1);
        }
    }

    for (int cnt_x=0;cnt_x<i_hight;++cnt_x)
    {   
        cnt =0;
        for (int cnt_y=0;cnt_y<pos_r-offset;++cnt_y)
        {
            mask_cr.at<Vec3f>(cnt_x,cnt_y)=Vec3f(1,1,1);
            mask_r.at<Vec3f>(cnt_x,cnt_y)=Vec3f(0,0,0);
        }

        for (int cnt_y=pos_r-offset;cnt_y<pos_r+offset;++cnt_y)
        {

            float a1=(cnt)/float(2*offset);
            float b1=(2*offset-cnt)/float(2*offset);
            // cout<<mask_l.at<Vec3f>(cnt_x,cnt_y)<<endl;
            mask_r.at<Vec3f>(cnt_x,cnt_y)=Vec3f(a1,a1,a1);
            mask_cr.at<Vec3f>(cnt_x,cnt_y)=Vec3f(b1,b1,b1);
            cnt++;

        }

        for (int cnt_y=pos_r+offset;cnt_y<3*i_width;++cnt_y)
        {
            mask_cr.at<Vec3f>(cnt_x,cnt_y)=Vec3f(0,0,0);
            mask_r.at<Vec3f>(cnt_x,cnt_y)=Vec3f(1,1,1);
        }
    }
    cout<<"construction"<<endl;
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

void Panorama::Trasf_rt(vector<Point2f> pts_d, vector<Point2f> pts_s, int mode)
{
    if (mode==0) //left
    {
        ptsShift(pts_d,i_width);ptsShift(pts_s,i_width);
        H_l = findHomography(pts_d, pts_s, RANSAC); // calculate perspective transform matrix
    }

    if (mode==2) //right
    {
        H_r = findHomography(pts_d, pts_s, RANSAC); // calculate perspective transform matrix
    }
}

Mat Panorama::Blend_tr(Mat& imgL, Mat& imgC, Mat& imgR)
{
    // Mat dst; 
    hconcat(Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgL,imgL);
    warpPerspective(imgL,dst_l,H_l,Size(imgL.cols*1,imgL.rows*1)); // apply perspective transform to image plane
    hconcat(imgR,Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgR);
    warpPerspective(imgR,dst_r,H_r, Size(imgR.cols*1,imgR.rows*1)); // apply perspective transform to image plane

    hconcat(imgC,Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgC);
    hconcat(Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),imgC,imgC);
    hconcat(dst_l,Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),dst_l);
    hconcat(Mat(i_hight,i_width,CV_8UC3,Scalar(0,0,0)),dst_r,dst_r);

    // mul.(mask) tasks long time to run... 
    dst_l.convertTo(dst_l , CV_32F, 1.0/255.0); 
    dst_l=dst_l.mul(mask_l);
    dst_l.convertTo(dst_l,CV_8U,255);

    dst_r.convertTo(dst_r , CV_32F, 1.0/255.0); 
    dst_r=dst_r.mul(mask_r);
    dst_r.convertTo(dst_r,CV_8U,255);

    imgC.convertTo(imgC , CV_32F, 1.0/255.0); 
    imgC=imgC.mul(mask_cl);
    imgC=imgC.mul(mask_cr);
    imgC.convertTo(imgC,CV_8U,255);

    img_p=dst_l+imgC+dst_r;

    resize(img_p, img_p, cv::Size( 2500, 1080 ));
    return img_p;
}


void Panorama::ptsShift(vector<Point2f>& pts_, int val_shift)
{
    for (int cnt_=0;cnt_<pts_.size();++cnt_)
    {
        pts_[cnt_].x+=val_shift;
    }
}
