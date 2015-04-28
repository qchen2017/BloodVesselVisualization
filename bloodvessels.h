#ifndef BLOODVESSELS_H
#define BLOODVESSELS_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QVector2D>
#include <QPointF>
#include <unordered_map>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;

namespace Ui {
class BloodVessels;
}

class BloodVessels : public QMainWindow
{
    Q_OBJECT

public:
    BloodVessels(QWidget *parent = 0);
    ~BloodVessels();

    bool isEmpty();
    void saveTipPoint(string imgpath, qreal x, qreal y);
    Mat identifyTip(Mat src, float x, float y);
    Mat displayTips(Mat src, string imName);
    void getManuallySelectedTips(unordered_map<string, QVector<QVector2D> > &tips);
    void tipsAnimation(unordered_map<string, QVector<QVector2D> > tips_map);
    QVector<Mat> getTipsImages();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BloodVessels *ui;

    void updateView(Mat imageOut);

    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    QVector<QVector2D> bloodVesselsTips;
    unordered_map<string, QVector<QVector2D> > bv_tips_map; // maps selected tips to image
    QVector<Mat> tips_images;

};

#endif // BLOODVESSELS_H
