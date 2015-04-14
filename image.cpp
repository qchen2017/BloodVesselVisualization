#include "image.h"
#include <QDebug>
#include <QVector3D>
#include <iostream>

using namespace cv;
using namespace std;

Mat Image::setImageView(Mat imageIn, int thresh_val, string mode)
{
    Mat imageOut;
    if (mode == "contour") {
        imageOut = setContour(imageIn, thresh_val);
    }

    return imageOut;
}

Mat Image::setContour(Mat src, int thresh_val)
{
     Mat src_gray;

    //convert to gray scale
    cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
    blur( src_gray, src_gray, Size(3,3) );
    threshold(src_gray, src_gray, thresh_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);

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

/************************************************************************************************************************/
/*The following codes are obtained from http://opencv-code.com/quick-tips/implementation-of-guo-hall-thinning-algorithm/*/
/************************************************************************************************************************/

void Image::thinningIteration(Mat &im, int iter)
{
    Mat marker = Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++) {
        for (int j = 1; j < im.cols-1; j++) {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0) {
                marker.at<uchar>(i,j) = 1;
            }
        }
    }

    im &= ~marker;
}

void Image::thinning(Mat &im)
{
    im /= 255;

    Mat prev = Mat::zeros(im.size(), CV_8UC1);
    Mat diff;

    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (countNonZero(diff) > 0);

    im *= 255;
}
