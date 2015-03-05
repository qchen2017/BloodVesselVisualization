#ifndef IMAGE_H
#define IMAGE_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace std;
using namespace cv;

class Image
{
public:
    Image() {}
    ~Image() {}

    Mat setImageView(Mat imageIn, int thresh_val, string mode);
    Mat setBackground(Mat imageIn);
    Mat removeOuterVessel(Mat imageIn);

private:
    Mat setContour(Mat src, int thresh_val);
    Mat setEdge(Mat src, int thresh_val);
    Mat setSkeleton(Mat img, int thresh_val);

    void thinningIteration(Mat &im, int iter);
    void thinning(Mat &im);
};

#endif // Image_H
