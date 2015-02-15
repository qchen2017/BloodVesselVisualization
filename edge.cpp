#include "imagemodes.h"

Edge::Edge()
{

}

Edge::~Edge()
{

}

Mat Edge::setImageView(Mat imageIn, int thresh_val)
{
    Mat imageOut = setEdge(imageIn, thresh_val);
    return imageOut;
}

Mat Edge::setEdge(Mat src, int thresh_val)
{
    Mat src_gray, edge;

    //gray scale
    cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
    blur(src_gray, edge, Size(3, 3));
    Canny(edge, edge, thresh_val, thresh_val*3, 3);

    return edge;
}
