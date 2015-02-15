#include "imagemodes.h"

using namespace std;

Contour::Contour()
{

}

Contour::~Contour()
{

}

Mat Contour::setImageView(Mat imageIn, int thresh_val)
{
    Mat imageOut = setContour(imageIn, thresh_val);
    return imageOut;
}


Mat Contour::setContour(Mat src, int thresh_val)
{
     Mat src_gray;

    //convert to gray scale
    cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
    blur( src_gray, src_gray, Size(3,3) );
    threshold(src_gray, src_gray, thresh_val, 255, cv::THRESH_BINARY_INV);

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(src_gray, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, Point(0, 0));
    Mat drawing = Mat::zeros(src_gray.size(), CV_8UC3 );
    for(int i = 0; i < (int)contours.size(); i++){
        Scalar color = Scalar(255,255,255);
        drawContours(drawing, contours, i, color, 3, 8, hierarchy, 0, Point());
    }
    return drawing;
}

