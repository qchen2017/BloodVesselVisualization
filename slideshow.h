#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QMainWindow>
#include <QBasicTimer>
#include <QGraphicsView>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace std;
using namespace cv;

namespace Ui {
class Slideshow;
}

class Slideshow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Slideshow(QWidget *parent = 0);
    ~Slideshow();

    void nextSlide();
    void setImageList(QStringList in, bool forTips);
    void timerEvent(QTimerEvent* event);
    void updateView(Mat imageOut);

    void tipsSlideshow(QVector<Mat> images, bool autoTipsFlag);

public slots:
    void on_playButton_clicked();
    void on_pauseButton_clicked();

private slots:
    void on_imageSlider_sliderMoved(int value);

private:
    Ui::Slideshow *ui;
    QBasicTimer interSlideTimer;
    int slideInterval; //delay time between slides
    int currentSlide;
    Mat src;
    bool tipsFlag;
    bool forAutomatedTips;
    QVector<Mat> tips_mats;

    QStringList imageList;
    QString imageName;
    cv::Mat imageOut_gray;
    QPixmap image;
    QGraphicsScene *scene;

};

#endif // SLIDESHOW_H
