#include "slideshow.h"
#include "ui_slideshow.h"
#include <iostream>

using namespace cv;
using namespace std;

bool paused = false;

Slideshow::Slideshow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Slideshow)
{
    ui->setupUi(this);
    slideInterval = 1000;
    currentSlide = 0;
    tipsFlag = false;
    forAutomatedTips = false;

    //connect(ui->imageSlider, SIGNAL(sliderMoved(int)),this, SLOT(on_imageSlider_moved(int)));

}

Slideshow::~Slideshow()
{
    delete ui;
}

void Slideshow::nextSlide()
{
    if (!tipsFlag) {
        imageName = imageList.at(currentSlide);
        src = imread(imageName.toStdString());

        //update slider position
        ui->imageSlider->setValue(currentSlide);

        //replay slideshow if last image is played
        if(currentSlide == (imageList.size()-1)) {
            currentSlide = 0;
        }
        else {
            ++currentSlide;
        }

        updateView(src);
    }

    else {
        src = tips_mats.at(currentSlide);
        ui->imageSlider->setValue(currentSlide);
        //replay slideshow if last image is played
        if(currentSlide == (tips_mats.size()-1)) {
            currentSlide = 0;
        }
        else {
            ++currentSlide;
        }

        updateView(src);
    }
}

void Slideshow::tipsSlideshow(QVector<Mat> images, bool autoTipsFlag)
{
    tips_mats = images;
    if (autoTipsFlag) {
        forAutomatedTips = true;
    }
    else {
        forAutomatedTips = false;
    }
    //imshow("Slideshow test", tips_mats.at(0));
}

//QStringList contains paths of opened images
void Slideshow::setImageList(QStringList in, bool forTips)
{
    imageList = in;
    ui->imageSlider->setMaximum(imageList.size()); //set max value of slider bar to # of images

    if (forTips) {
        tipsFlag = true;
    }
    else {
        tipsFlag = false;
    }
}

//automatically called when timer goes off (ie. when slideInterval = 0)
void Slideshow::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);

    if(!paused)
        nextSlide();
}

//update view screen in widget
void Slideshow::updateView(Mat imageOut)
{
    if (forAutomatedTips) {
        QImage img((uchar*)imageOut.data, imageOut.cols, imageOut.rows, QImage::Format_Indexed8);
        image = QPixmap::fromImage(img);
    }
    else {
        cvtColor(imageOut, imageOut_gray, cv::COLOR_BGR2GRAY);
        blur(imageOut_gray, imageOut_gray, Size(3,3));
        QImage img((uchar*)imageOut_gray.data, imageOut_gray.cols, imageOut_gray.rows, QImage::Format_Indexed8);
        image = QPixmap::fromImage(img);
    }

    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());

    //fit image to screen and display
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
    ui->graphicsView->setScene(scene);
}

//stop slideshow timer when the window is closed
void Slideshow::closeEvent(QCloseEvent *event) {
    interSlideTimer.stop();
    close(); //closes this widget
}

/*
 * Slideshow UI functions below
*/

void Slideshow::on_imageSlider_sliderMoved(int value)
{
    paused = true;
    currentSlide = value;
    nextSlide();
}

void Slideshow::on_playButton_clicked()
{
    paused = false;
    interSlideTimer.start(slideInterval, this);
    nextSlide();

}

void Slideshow::on_pauseButton_clicked()
{
    paused = true;
}

void Slideshow::on_speedSlider_sliderMoved(int value)
{
    interSlideTimer.start(value, this); //restart timer with new timeout value
    ui->slideSpeed_LineEdit->setText(QString::number(value/1000.0) + " sec"); //show delay in seconds
}
