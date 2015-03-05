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

    Mat identifyTip(Mat src, float x, float y);
    bool isEmpty();
    void saveTipPoint(qreal x, qreal y);
    Mat displayTips(Mat src);
    Mat detectTips(Mat imageIn);

//    void detectROI(Mat imageIn);
//    void detectSeedPoints(Mat src);
//    void enhance(Mat src);
//    void detectCenter(Mat src);

    void testTips();
    void testTips2();

    void endp(Mat &src, Mat &dst);
    void skel(Mat &src, Mat &dst);
    void applylut_8(Mat &src, Mat &dst, Mat &lut);
    void applylut_1(Mat &src, Mat &dst);
    void GetLutSkel(Mat &Lut);
private:
    QVector<QVector2D> bloodVesselsTips;

};

#endif // BLOODVESSELS_H
