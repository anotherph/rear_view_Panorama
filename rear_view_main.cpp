#include <opencv2/opencv.hpp>
#include "./rear_view_h.h"
#include <fstream>
#define left 0
#define center 1
#define right 2

using namespace cv;
using namespace std;

int main()
{
    // load images and pts

    Mat img_srcL= imread(".././Img/panorama_d/img_original/left.jpg"); 
    Mat img_srcR= imread(".././Img/panorama_d/img_original/right.jpg");
    Mat img_srcC= imread(".././Img/panorama_d/img_original/center.jpg");
    string  dir_pts_left = ".././Img/panorama_d/pts/left.txt"; 
    string  dir_pts_center_l = ".././Img/panorama_d/pts/center_l.txt";
    string  dir_pts_right = ".././Img/panorama_d/pts/right.txt"; 
    string  dir_pts_center_r = ".././Img/panorama_d/pts/center_r.txt";
    vector<Point2f> m_pts_left; 
    vector<Point2f> m_pts_center_l;     
    vector<Point2f> m_pts_right;     
    vector<Point2f> m_pts_center_r;

    Panorama *panorama = new Panorama();
    m_pts_left=panorama->readTxtFile(dir_pts_left);
    m_pts_center_l=panorama->readTxtFile(dir_pts_center_l);
    m_pts_right=panorama->readTxtFile(dir_pts_right);
    m_pts_center_r=panorama->readTxtFile(dir_pts_center_r);

    Mat img_dstL = panorama->Trasf(img_srcL,img_srcC,m_pts_left,m_pts_center_l,left);
    Mat img_dstR = panorama->Trasf(img_srcR,img_srcC,m_pts_right,m_pts_center_r,right);
    Mat img_pano = panorama->Blend(img_dstL,img_srcC,img_dstR);

    imwrite(".././Img/panorama_d/pts/pano.jpg", img_pano);
    
    return 0;

}