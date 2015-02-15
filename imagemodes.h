#ifndef IMAGEMODES_H
#define IMAGEMODES_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;

class ImageModes
{
public:
    ImageModes() {}
    ~ImageModes() {}

    virtual Mat setImageView(Mat imageIn, int thresh_val) = 0;
};

class Contour: public ImageModes
{
public:
    Contour();
    ~Contour();

    Mat setImageView(Mat imageIn, int thresh_val);

private:
    Mat setContour(Mat src, int thresh_val);

};

class Edge: public ImageModes
{
public:
    Edge();
    ~Edge();

    Mat setImageView(Mat imageIn, int thresh_val);

private:
    Mat setEdge(Mat src, int thresh_val);

};

#endif // IMAGEMODES_H
