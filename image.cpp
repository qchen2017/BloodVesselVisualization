#include "image.h"
#include <QDebug>
#include <QVector3D>
#include <iostream>

using namespace std;

Mat Image::setImageView(Mat imageIn, int thresh_val, string mode)
{
    Mat imageOut;
    if (mode == "contour") {
        imageOut = setContour(imageIn, thresh_val);
    }
    else if (mode == "edge") {
        imageOut = setEdge(imageIn, thresh_val);
    }
    else if (mode == "skeleton") {
        imageOut = setSkeleton(imageIn, thresh_val);
    }
    return imageOut;
}

Mat Image::setBackground(Mat imageIn)
{
    Mat imageOut = imageIn;

    // left side
    for(int y = 0; y < imageIn.rows; y++) {
        for(int x = 0; x < 150; x++) {
            // get pixel
            Vec3b color = imageOut.at<Vec3b>(Point(x,y));
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            imageOut.at<Vec3b>(Point(x,y)) = color;
        }
    }

    // right side
    for(int y = 0; y < imageIn.rows; y++) {
        for(int x = (imageIn.cols - 150); x < imageIn.cols; x++) {
            // get pixel
            Vec3b color = imageOut.at<Vec3b>(Point(x,y));
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            imageOut.at<Vec3b>(Point(x,y)) = color;
        }
    }

    // lower left
    for(int y = (imageIn.rows - 300); y < imageIn.rows; y++) {
        for(int x = 150; x < 400; x++) {
            // get pixel
            Vec3b color = imageOut.at<Vec3b>(Point(x,y));
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            imageOut.at<Vec3b>(Point(x,y)) = color;
        }
    }

    // lower right
    for(int y = (imageIn.rows - 300); y < imageIn.rows; y++) {
        for(int x = (imageIn.cols - 400); x < (imageIn.cols - 150); x++) {
            // get pixel
            Vec3b color = imageOut.at<Vec3b>(Point(x,y));
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            imageOut.at<Vec3b>(Point(x,y)) = color;
        }
    }

    return imageOut;
}

Mat Image::removeOuterVessel(Mat imageIn)
{
    Mat imageOut = imageIn;

    for (int y = 0; y < imageIn.rows; y++) {
        int x = 0;
        Vec3b color = imageOut.at<Vec3b>(Point(x,y));
        QVector3D colorVal(color[0], color[1], color[2]);

        // from the left side
        while ((colorVal != QVector3D(255, 255, 255)) && ((x < 500) || y < 500)) {
            x++; // continue moving across the columns
            color = imageOut.at<Vec3b>(Point(x,y));
            colorVal.setX(color[0]);
            colorVal.setY(color[1]);
            colorVal.setZ(color[2]);
        }

        for (int i = x; i < x + 75; i++) {
            color = imageOut.at<Vec3b>(Point(i,y));
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            imageOut.at<Vec3b>(Point(i,y)) = color;
        }

        // from the right side
        x = imageIn.cols;
        color = imageOut.at<Vec3b>(Point(x,y));
        colorVal.setX(color[0]);
        colorVal.setY(color[1]);
        colorVal.setZ(color[2]);
        while ((colorVal != QVector3D(255, 255, 255)) && ((x > imageIn.cols - 500) || (y < 500))) {
            x--;
            color = imageOut.at<Vec3b>(Point(x,y));
            colorVal.setX(color[0]);
            colorVal.setY(color[1]);
            colorVal.setZ(color[2]);
        }

        for (int i = x; i > x - 75; i--) {
            color = imageOut.at<Vec3b>(Point(i,y));
            color[0] = 0;
            color[1] = 0;
            color[2] = 0;
            imageOut.at<Vec3b>(Point(i,y)) = color;
        }

    }

    return imageOut;

}

Mat Image::detectWhiteSpots(Mat imageIn)
{
    Mat imageOut = imageIn;



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

Mat Image::setEdge(Mat src, int thresh_val)
{
    Mat src_gray, edge;

    //gray scale
    cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, edge, Size(3, 3));
    Canny(edge, edge, thresh_val, thresh_val*3, 3);

    return edge;
}

Mat Image::setSkeleton(Mat img, int thresh_val)
{
    Mat src = cv::imread("/Users/trishamariefuntanilla/Downloads/test.jpg");

    Mat bw;
    cvtColor(src, bw, COLOR_BGR2GRAY);
    threshold(bw, bw, 133, 255, cv::THRESH_BINARY);

    imshow ("bw", bw);

    thinning(bw);

    imshow("src", src);
    imshow("dst", bw);
    waitKey(0);

//    Mat dst;
//    cvtColor(src, dst, cv::COLOR_BGR2GRAY);
//    threshold(dst, dst, thresh_val, 255, cv::THRESH_BINARY);
//    thinning(dst);
//    return dst;
    return src;
}

void Image::thinningIteration(Mat &im, int iter)
{
    Mat marker = Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
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

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
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
