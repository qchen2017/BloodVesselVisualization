#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QMainWindow>
#include <QBasicTimer>
#include <QGraphicsView>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

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
    void setImageList(QStringList);
    void timerEvent(QTimerEvent*);
    void updateView(cv::Mat);
    void closeEvent(QCloseEvent*);

public slots:
    void imageSlider_moved(int);
    void speedSlider_moved(int);
    void on_playButton_clicked();
    void on_pauseButton_clicked();

private:
    Ui::Slideshow *ui;
    QBasicTimer interSlideTimer;
    int slideInterval; //delay time between slides
    int currentSlide;
    int numSlides; //number of slides
    cv::Mat src;

    QStringList imageList;
    QString imageName;
    cv::Mat imageOut_gray;
    QPixmap image;
    QGraphicsScene *scene;
};

#endif // SLIDESHOW_H
