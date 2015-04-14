#ifndef IMAGE_H
#define IMAGE_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;

class Image
{
public:
    Image() {}
    ~Image() {}

    Mat setImageView(Mat imageIn, int thresh_val, string mode); // set up image for changing modes

private:
    Mat setContour(Mat src, int thresh_val); // contour mode

    // skeleton mode - currently not implemented on the ui
    void thinning(Mat &im);
    void thinningIteration(Mat &im, int iter);

};

#endif // Image_H
