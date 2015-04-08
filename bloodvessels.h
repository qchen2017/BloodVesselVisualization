#ifndef BLOODVESSELS_H
#define BLOODVESSELS_H

#include <QVector>
#include <QVector2D>
#include <QPointF>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;

class BloodVessels
{
public:
    BloodVessels();
    ~BloodVessels();

    bool isEmpty();

    /* Debugging functions*/
    void saveTipPoint(qreal x, qreal y);
    Mat displayTips(Mat src);

private:
    QVector<QVector2D> bloodVesselsTips;

};

#endif // BLOODVESSELS_H
