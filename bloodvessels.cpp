#include "bloodvessels.h"
#include <QVector3D>
#include <QDebug>
#include <iostream>

using namespace std;

BloodVessels::BloodVessels()
{

}

BloodVessels::~BloodVessels()
{

}

Mat BloodVessels::identifyTip(Mat src, float x, float y)
{
    Point dot = Point(x, y);
    int thickness = -1;
    int lineType = 8;

     circle(src,
            dot,
            25.0,
            Scalar(0, 0, 255),
            thickness,
            lineType);

     return src;
}

bool BloodVessels::isEmpty()
{
    if (bloodVesselsTips.size() == 0) {
        return true;
    }
    return false;
}

void BloodVessels::saveTipPoint(qreal x, qreal y)
{
    bloodVesselsTips.push_back(QVector2D(x, y));
    qDebug() << QVector2D(x, y);
}

Mat BloodVessels::displayTips(Mat src)
{
    Mat imageOut = src;
    int thickness = -1;
    int lineType = 8;

    for (int i = 0; i < bloodVesselsTips.size(); i++) {
        float x = (float) bloodVesselsTips[i].x();
        float y = (float) bloodVesselsTips[i].y();
        Point dot = Point(x, y);
        circle(src,
               dot,
               25.0,
               Scalar(0, 0, 255),
               thickness,
               lineType);
    }

    return imageOut;
}

Mat BloodVessels::detectTips(Mat imageIn)
{
    Mat imageOut = imageIn;
    int thickness = -1;
    int lineType = 8;

    for (int y = 0; y < 300; y++) {
        int x = 0;
        Vec3b color = imageOut.at<Vec3b>(Point(x,y));
        QVector3D colorVal(color[0], color[1], color[2]);

        // from the left side
        while ((colorVal != QVector3D(255, 255, 255)) && ((x < 300) || y < 500)) {
            x++; // continue moving across the columns
            color = imageOut.at<Vec3b>(Point(x,y));
            colorVal.setX(color[0]);
            colorVal.setY(color[1]);
            colorVal.setZ(color[2]);
        }

        Point dot = Point(x, y);
        circle(imageOut,
               dot,
               25.0,
               Scalar(0, 0, 255),
               thickness,
               lineType);

        // from the right side
        x = imageIn.cols;
        color = imageOut.at<Vec3b>(Point(x,y));
        colorVal.setX(color[0]);
        colorVal.setY(color[1]);
        colorVal.setZ(color[2]);
        while ((colorVal != QVector3D(255, 255, 255)) && ((x > imageIn.cols - 300) || (y < 500))) {
            x--;
            color = imageOut.at<Vec3b>(Point(x,y));
            colorVal.setX(color[0]);
            colorVal.setY(color[1]);
            colorVal.setZ(color[2]);
        }

        dot = Point(x, y);
        circle(imageOut,
               dot,
               25.0,
               Scalar(0, 0, 255),
               thickness,
               lineType);

    }

    return imageOut;
}

//void BloodVessels::detectROI(Mat imageIn)
//{
//    Mat src_gray;

//    cvtColor(imageIn, src_gray, cv::COLOR_BGR2GRAY );

//    GaussianBlur( src_gray, src_gray, cv::Size(9, 9), 2, 2 );

//    vector<Vec3f> circles;

//    HoughCircles(src_gray, circles, HOUGH_GRADIENT,
//                 2,   // accumulator resolution (size of the image / 2)
//                 5,  // minimum distance between two circles
//                 100, // Canny high threshold
//                 100, // minimum number of votes
//                 0, 1000); // min and max radius

//    cout << circles.size() << endl;
//    cout << "end of test" << endl;

//    vector<cv::Vec3f>::
//            const_iterator itc= circles.begin();

//    while (itc!=circles.end()) {

//        cv::circle(src_gray,
//                   cv::Point((*itc)[0], (*itc)[1]), // circle centre
//                (*itc)[2],       // circle radius
//                cv::Scalar(255), // color
//                2);              // thickness

//        ++itc;
//    }
//    namedWindow("image", WINDOW_AUTOSIZE);
//    imshow("image",src_gray);

//    //return src_gray;


//} // detect region of interest

//void BloodVessels::detectSeedPoints(Mat src)
//{
//    enhance(src);
//    detectCenter(src);
//}

//void BloodVessels::enhance(Mat src)
//{
//    // convert to gray scale
//    Mat src_gray;
//    cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
//    equalizeHist(src_gray, src_gray);
//    //blur(src_gray, src_gray, Size(3,3));
//    //threshold(src_gray, src_gray, thresh_val, 255, cv::THRESH_BINARY_INV);


//    namedWindow("Gray image", WINDOW_NORMAL);
//    resizeWindow("Gray image", 300, 300);
//    imshow("Gray image", src_gray);

//}

//void BloodVessels::detectCenter(Mat src)
//{

//}

void BloodVessels::testTips()
{
    bloodVesselsTips.clear();

    bloodVesselsTips.push_back(QVector2D(1345.94, 364.606));
    bloodVesselsTips.push_back(QVector2D(659.394, 577.939));
    bloodVesselsTips.push_back(QVector2D(1024, 1776.48));
    bloodVesselsTips.push_back(QVector2D(643.879, 1454.55));
    bloodVesselsTips.push_back(QVector2D(1442.91, 1625.21));
    bloodVesselsTips.push_back(QVector2D(1803.64, 1276.12));
    bloodVesselsTips.push_back(QVector2D(1621.33, 1567.03));
    bloodVesselsTips.push_back(QVector2D(411.152, 977.455));
    bloodVesselsTips.push_back(QVector2D(411.152, 1268.36));
    bloodVesselsTips.push_back(QVector2D(857.212, 1838.55));
    bloodVesselsTips.push_back(QVector2D(756.364, 1664));
    bloodVesselsTips.push_back(QVector2D(1788.12, 694.303));
}

void BloodVessels::testTips2()
{
    bloodVesselsTips.clear();

    bloodVesselsTips.push_back(QVector2D(1338.18, 174.545));
    bloodVesselsTips.push_back(QVector2D(814.545, 104.727));
    bloodVesselsTips.push_back(QVector2D(876.606, 387.879));
    bloodVesselsTips.push_back(QVector2D(190.061, 1105.45));
    bloodVesselsTips.push_back(QVector2D(310.303, 1493.33));
    bloodVesselsTips.push_back(QVector2D(430.545, 1629.09));
    bloodVesselsTips.push_back(QVector2D(1004.61, 1916.12));
    bloodVesselsTips.push_back(QVector2D(1551.52, 1985.94));
    bloodVesselsTips.push_back(QVector2D(1869.58, 1307.15));
    bloodVesselsTips.push_back(QVector2D(1768.73, 1757.09));
    bloodVesselsTips.push_back(QVector2D(667.151, 1881.21));
    bloodVesselsTips.push_back(QVector2D(477.091, 911.515));
    bloodVesselsTips.push_back(QVector2D(1900.61, 1124.85));
    bloodVesselsTips.push_back(QVector2D(1989.82, 911.515));
    bloodVesselsTips.push_back(QVector2D(1997.58, 1047.27));
    bloodVesselsTips.push_back(QVector2D(2183.76, 1330.42));
    bloodVesselsTips.push_back(QVector2D(1978.18, 1388.61));
}
