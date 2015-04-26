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

    connect(ui->imageSlider, SIGNAL(sliderMoved(int)),this, SLOT(on_imageSlider_moved(int)));

}

Slideshow::~Slideshow()
{
    delete ui;
}


void Slideshow::nextSlide() {
    imageName = imageList.at(currentSlide);
    src = imread(imageName.toStdString());

    //update slider position
    ui->imageSlider->setValue(currentSlide);

    //replay slideshow if last image is played
    if(currentSlide == (imageList.size()-1))
        currentSlide = 0;
    else
        ++currentSlide;

    updateView(src);
}

//QStringList contains paths of opened images
void Slideshow::setImageList(QStringList in) {
  imageList = in;
  ui->imageSlider->setMaximum(imageList.size()-1); //set max value of slider bar to # of images
}

//automatically called when timer goes off (ie. when slideInterval = 0)
void Slideshow::timerEvent(QTimerEvent* event) {
    Q_UNUSED(event);

    if(!paused)
        nextSlide();
}

//update view screen in widget
void Slideshow::updateView(Mat imageOut) {

   cvtColor(imageOut, imageOut_gray, cv::COLOR_BGR2GRAY);
   blur( imageOut_gray, imageOut_gray, Size(3,3) );
   QImage img((uchar*)imageOut_gray.data, imageOut_gray.cols, imageOut_gray.rows, QImage::Format_Indexed8);
   image = QPixmap::fromImage(img);

   scene = new QGraphicsScene(this);
   scene->addPixmap(image);
   scene->setSceneRect(0, 0, image.width(), image.height());

   //fit image to screen and display
   ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
   ui->graphicsView->setAlignment(Qt::AlignCenter);
   ui->graphicsView->setScene(scene);

}


/*
 * Slideshow UI functions below
*/

void Slideshow::on_imageSlider_moved(int value) {
    paused = true;
    currentSlide = value;
    nextSlide();
}

void Slideshow::on_playButton_clicked() {
    paused = false;
    interSlideTimer.start(slideInterval, this);
    nextSlide();
}

void Slideshow::on_pauseButton_clicked() {
    paused = true;
}


