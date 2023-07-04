// combine images corresponding to one-points (0,0,0)

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

class Stitching360
{
private:

    cv::Size                                m_szBoard;	            /**** The number of corner points in each row and column on the calibration board       ****/
    cv::Vec3d                               m_vTemp; 
    std::vector<cv::Point2f>                n_vCorners;             /**** Cache the corners detected on each image ****/

public:
    Stitching360();
    ~Stitching360();

    virtual int Init(int nSrcHeight, int nSrcWidth);
    virtual Vec3d findCorners(Mat &image); 
};

std::vector<cv::Point2f> vecTemp;
void OnMouseAction(int event, int x, int y, int flags, void *para)
{
    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        vecTemp.push_back(cv::Point2f(x, y));
    }
}

void MatBlending(int x, int y, Mat &Mat1, Vec2i &Vec1, Mat &total_)
{
    int size_x = x;
    int size_y = y;
    Mat mat1=Mat1;
    Vec2i vec1=Vec1;

    for (int y = 0; y < mat1.cols; y++) {
	    for (int x = 0; x < mat1.rows; x++) {
            if (total_.at<Vec3b>(size_x/2-vec1(1)+x, size_y/2-vec1(0)+y)==Vec3b(0,0,0))
                total_.at<Vec3b>(size_x/2-vec1(1)+x, size_y/2-vec1(0)+y) = mat1.at<Vec3b>(x,y);
            else
            {
                total_.at<Vec3b>(size_x/2-vec1(1)+x, size_y/2-vec1(0)+y)+=mat1.at<Vec3b>(x,y);
                total_.at<Vec3b>(size_x/2-vec1(1)+x, size_y/2-vec1(0)+y)/=2;
            }
	    }
    }
}

int main ()
{
    // load images

    Mat left, right, center; 
    
    // left = imread("../.././Img/left.jpg"); // for cmake building
    // right = imread("../.././Img/right.jpg");
    // center = imread("../.././Img/center.jpg");

    left = imread(".././Img/left.jpg");
    right = imread(".././Img/right.jpg");
    center = imread(".././Img/center.jpg");

    Vec3d p_left, p_right, p_center; // [x, y, l] 2D point(x,y) indicates (0,0) in world coordinate and l: length of two adjacent points 
    Vec2i p_left_, p_right_, p_center_;

    Stitching360 *Image_ = new Stitching360();
    Image_->Init(left.cols, left.rows);
    
    // find chessboard points and get p_left, p_right, p_center

    p_left=Image_->findCorners(left);
    p_center=Image_->findCorners(center);
    p_right=Image_->findCorners(right);

    double scale_l=p_left(2)/p_center(2);
    double scale_r=p_right(2)/p_center(2);

    p_left_(0)=int(p_left(0)/scale_l); p_left_(1)=int(p_left(1)/scale_l);
    p_right_(0)=int(p_right(0)/scale_r); p_right_(1)=int(p_right(1)/scale_r);
    p_center_(0)=int(p_center(0));p_center_(1)=int(p_center(1));

    // match the scale of images 

    Mat left_, right_, center_;
    resize(left,left_,Size(int(left.cols/scale_l),int(left.rows/scale_l)));
    resize(right,right_,Size(int(right.cols/scale_r),int(right.rows/scale_r)));
    center_=center;

    // combine images corresponding to (0,0)

    int size_x = 2000;
    int size_y = 3000;

    Mat total_(size_x,size_y,CV_8UC3);

    MatBlending(size_x,size_y,left_, p_left_, total_);
    MatBlending(size_x,size_y,right_, p_right_, total_);
    MatBlending(size_x,size_y,center_, p_center_, total_);

    circle(total_,Point(size_y/2,size_x/2),5, Scalar(255,0,255),10,-1,0);
    
    //  while(1)
    //  {
    //     imshow("1",total_);
    //     if (waitKey(1) == 27)
	// 		break; 
    //  }
     
    // imwrite("../.././Img/total1.jpg", total_); // for cmake building
    imwrite(".././Img/total1.jpg", total_);

    return 0;
}


Stitching360::Stitching360():m_szBoard(cv::Size(4,4)) { 
    // edit the image directory and calibration board var
}

Stitching360::~Stitching360() 
{
    cv::destroyAllWindows();
}

int Stitching360::Init(int nSrcHeight, int nSrcWidth)
{
    return 1;
}

Vec3d Stitching360::findCorners(Mat &image) 
{
    /* Extract corner points */
    cv::Mat imageGray;
    cvtColor(image, imageGray, CV_RGB2GRAY);
    /*Input image, number of corners, detected corners, adjustments made to the image before finding corners*/
    bool patternfound = cv::findChessboardCorners(image, m_szBoard, n_vCorners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE +
        cv::CALIB_CB_FAST_CHECK);

    if (!patternfound)
    {
        std::cout << "Corner not found, need to be selected manually in figure" << std::endl;

        imshow("image", image);
        cv::waitKey(1);
        while(1)
        {
            int key = cv::waitKey(10);
            setMouseCallback("image", OnMouseAction);
            if (key == 'q')
                break;
        }
        n_vCorners = vecTemp;
        vecTemp.clear();
        cv::destroyWindow("image");
        m_vTemp(0)=n_vCorners[0].x;
        m_vTemp(1)=n_vCorners[0].y;
        m_vTemp(2)=sqrt(pow(n_vCorners[0].x-n_vCorners[1].x,2)+pow(n_vCorners[0].y-n_vCorners[1].y,2));

    }
    else
    {
        /* Sub-pixel precision, refine the detected integer coordinate corner points, the refined points are stored in corners, the least square iteration is 100 times, the error is 0.001*/
        cornerSubPix(imageGray, n_vCorners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 100, 0.001));
        m_vTemp(0)=n_vCorners[0].x;
        m_vTemp(1)=n_vCorners[0].y;
        m_vTemp(2)=sqrt(pow(n_vCorners[0].x-n_vCorners[1].x,2)+pow(n_vCorners[0].y-n_vCorners[1].y,2));
    }

    return m_vTemp;
}
