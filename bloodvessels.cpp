#include "bloodvessels.h"
#include <QVector3D>
#include <QDebug>
#include <iostream>

using namespace cv;
using namespace std;

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
               20.0,
               Scalar(0, 0, 255),
               thickness,
               lineType);
    }

    return imageOut;
}
