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
    ui->imageSlider->setValue(0); //set slider at beginning
    ui->speedSlider->setMaximum(3000); //set max speed to 3 seconds
    ui->speedSlider->setMinimum(1);
    ui->speedSlider->setValue(1000); //initialize slider at position 1000

    slideInterval = 1000; //1 second
    ui->slideSpeed_LineEdit->setText(QString::number(slideInterval/1000) + " sec"); //set speed line edit

    currentSlide = 0;

    connect(ui->imageSlider, SIGNAL(sliderMoved(int)),this, SLOT(imageSlider_moved(int)));
    connect(ui->speedSlider, SIGNAL(sliderMoved(int)),this, SLOT(speedSlider_moved(int)));
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

    //update slide number on line edit
    ui->slideNum_LineEdit->setText(QString::number(currentSlide+1) + "/" + QString::number(numSlides+1) );

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
  numSlides = imageList.size() - 1;

  ui->imageSlider->setMaximum(numSlides); //set max value of slider bar to # of images
  ui->slideNum_LineEdit->setText(QString::number(currentSlide) + "/" + QString::number(numSlides+1) );
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

//stop slideshow timer when the window is closed
void Slideshow::closeEvent(QCloseEvent *event) {
    interSlideTimer.stop();
    close(); //closes this widget
}

/*
 * Slideshow UI functions below
 */

void Slideshow::imageSlider_moved(int value) {
    paused = true;
    currentSlide = value;
    nextSlide();
}

void Slideshow::speedSlider_moved(int value) {
    interSlideTimer.start(value, this); //restart timer with new timeout value
    ui->slideSpeed_LineEdit->setText(QString::number(value/1000.0) + " sec"); //show delay in seconds
}

void Slideshow::on_playButton_clicked() {
    paused = false;
    interSlideTimer.start(slideInterval, this);
    nextSlide();
}

void Slideshow::on_pauseButton_clicked() {
    paused = true;
}


