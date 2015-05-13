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
    bool thisImageTipsIsEmpty(string imgpath);
    void saveTipPoint(string imgpath, qreal x, qreal y);
    void deleteTipPoint(string imgpath);
    void deleteAllTipPoints(string imgpath);
    cv::Mat identifyTip(cv::Mat src,float x, float y, int tips_size, QColor tips_color, QPointF refPtPixel);
    cv::Mat displayTips(cv::Mat src, string imName, int tips_size, QColor tips_color, QPointF refPtPixel, bool lengthEnabled, bool tipsEnabled);
    void getManuallySelectedTips(unordered_map<string, QVector<QVector2D> > &tips);
    void tipsAnimation(unordered_map<string, QVector<QVector2D> > tips_map);
    QVector<cv::Mat> getTipsImages();  
    QVector<cv::Mat> getTipsImagesWithOrigBG();
    void clearImageVectors();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::BloodVessels *ui; // currently not used

    void updateView(cv::Mat imageOut);

    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    QVector<QVector2D> bloodVesselsTips;
    unordered_map<string, QVector<QVector2D> > bv_tips_map; // maps selected tips to image
    QVector<cv::Mat> tips_images;
    QVector<cv::Mat> tips_images_orig_bg;

};

#endif // BLOODVESSELS_H
