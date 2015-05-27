#ifndef IMAGE_H
#define IMAGE_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace std;

class Image
{
public:
    Image() {}
    ~Image() {}

    cv::Mat setImageView(cv::Mat imageIn, int thresh_val, string mode); // set up image for changing modes

private:
    cv::Mat setContour(cv::Mat src, int thresh_val); // contour mode

};

#endif // Image_H
