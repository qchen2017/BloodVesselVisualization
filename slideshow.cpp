#include "slideshow.h"
#include "ui_slideshow.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <iostream>
#include <QDebug>

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
    ui->actionSave->setDisabled(true);

    ui->imageSlider->setValue(0); //set slider at beginning
    ui->speedSlider->setMaximum(3000); //set max speed to 3 seconds
    ui->speedSlider->setMinimum(1);
    ui->speedSlider->setValue(slideInterval); //initialize slider at position 1000
    ui->slideSpeed_LineEdit->setText(QString::number(slideInterval/1000) + " sec"); //set speed line edit
}

Slideshow::~Slideshow()
{
    delete ui;
}

void Slideshow::nextSlide()
{
    //qDebug() << tipsFlag;
    if (!tipsFlag) {
        imageName = imageList.at(currentSlide);
        src = imread(imageName.toStdString());

        //update slider position
        ui->imageSlider->setValue(currentSlide);

        //update slide number on line edit
        ui->slideNum_LineEdit->setText(QString::number(currentSlide+1) + "/" + QString::number(numSlides));

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

        //update slide number on line edit
        ui->slideNum_LineEdit->setText(QString::number(currentSlide+1) + "/" + QString::number(numSlides));

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
        ui->actionSave->setDisabled(true);
        forAutomatedTips = true;
    }
    else {
        ui->actionSave->setEnabled(true); // only allow saving for manual tips animation
        forAutomatedTips = false;
    }
    //imshow("Slideshow test", tips_mats.at(0));
}

//QStringList contains paths of opened images
void Slideshow::setImageList(QStringList in, bool forTips)
{

    imageList = in;
    numSlides = imageList.size();
    ui->imageSlider->setMaximum(numSlides); //set max value of slider bar to # of images
    ui->slideNum_LineEdit->setText(QString::number(currentSlide) + "/" + QString::number(numSlides) );

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

    // reset everything before closing widget
    interSlideTimer.stop();
    slideInterval = 1000;
    currentSlide = 0;
    tipsFlag = false;
    forAutomatedTips = false;
    imageList.clear();
    tips_mats.clear();
    ui->speedSlider->setValue(1000);
    ui->slideSpeed_LineEdit->setText(QString::number(1000/1000) + " sec");

    close(); //closes this widget
    event->accept();
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

void Slideshow::on_actionSave_triggered()
{
    // manually select a location to save the .avi
    QString savePath = QFileDialog::getSaveFileName(this, tr("Save File"), "",tr("AVI (*.avi)"));

    if (!tipsFlag) {
        //find framesize
        QString frameName = imageList.at(0);
        Mat frame = imread(frameName.toStdString());
        Size s = frame.size();
        double height = s.height;
        double width = s.width;

        Size frameSize(static_cast<int>(width), static_cast<int>(height));
        VideoWriter outVideoFile (savePath.toStdString(), -1, 1, frameSize, true);

        for(int i = 0 ; i < imageList.size(); i++) { //display images/frames to MyVideo, and create output video
            frameName = imageList.at(i);
            frame = imread(frameName.toStdString());

            if (!frame.data) { // Check for invalid input
                cout <<  "Could not open or find the image at " << i << endl ;
                return;
            }

            outVideoFile << (frame); //write the frame into the file
        }
    }
    else {
        Mat frame = tips_mats.at(0);
        Size s = frame.size();
        double height = s.height;
        double width = s.width;

        Size frameSize(static_cast<int>(width), static_cast<int>(height));

        //initialize videowriter
        //constructor format: Location & name of output file, fourcc codec, framerate (# frames/sec), framesize, isColor

//        VideoWriter outVideoFile (savePath.toStdString(), -1, 1, frameSize, true);

        VideoWriter outVideoFile (savePath.toStdString(), -1, 2, frameSize, true);


        if (!outVideoFile.isOpened()) { // check if the fourcc is allowed
            QMessageBox::information(this, "ERROR!", "ERROR: Failed to write the video");
            // cout << "ERROR: Failed to write the video" << endl;
            return;
        }

        for (int i = 0; i < tips_mats.size(); i++) {
            frame = tips_mats.at(i);

            if (!frame.data) { // check for invalid input
                QString error = "Could not open or find the image at ";
                QString num; num.setNum(i);
                error.append(num);
                QMessageBox::information(this, "ERROR!", error);
                // cout <<  "Could not open or find the image at " << i << endl ;
                return;
            }

            outVideoFile << (frame);
            waitKey(100);
        }
    }
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
