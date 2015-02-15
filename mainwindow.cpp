#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->threshold_lineEdit->setText("0");

    contour = new Contour();
    edge = new Edge();

    threshold_val = 0;
    scaleFactor = 1.15;
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* Main Menu Functions */

void MainWindow::errorMsg()
{
    QMessageBox::about(this, tr("Error !"),
                       tr("No image is open.\n"
                          "Please open an image first.\n"));
}// error no image

bool MainWindow::check_imageOpened()
{
    if(imagePath == NULL)
        return false;
    return true;
}// check if image open

void MainWindow::on_actionOpen_triggered()
{
    imagePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "",
                                                    tr("Images (*.png *.xpm *.jpg)"));

    if (imagePath == NULL) {
        errorMsg();
        return;
    }

     //set source image
    src = imread(imagePath.toStdString());
    cv::resize(src, src, cv::Size2i(src.cols/3, src.rows/3));

    //set graphic view
    imageObject = new QImage();
    imageObject->load(imagePath);
    image = QPixmap::fromImage(*imageObject);
    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());
    ui->graphicsView->setScene(scene);

    // fit to window by default
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->setAlignment(Qt::AlignCenter);

}// open image

void MainWindow::on_actionSave_triggered()
{
    imagePath = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                                                     tr("JPEG (*.jpg *.jpeg);;PNG (*.png)"));

    *imageObject = image.toImage();
    imageObject->save(imagePath);

}// save image

void MainWindow::on_actionFit_to_Window_triggered()
{
    if(scene == NULL){
        errorMsg();
        return;
    }

    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
}

void MainWindow::on_actionActual_Size_triggered()
{
    if(scene == NULL){
        errorMsg();
        return;
    }

}

void MainWindow::on_actionZoom_In__triggered()
{
    if (scene == NULL){
        errorMsg();
        return;
    }

    ui->graphicsView->scale(scaleFactor, scaleFactor);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    if (scene == NULL){
        errorMsg();
        return;
    }

    ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}

/**********************************************************************************/
/**********************************************************************************/

/* User interface controls */

void MainWindow::on_threshold_horizontalSlider_valueChanged(int value)
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    }// error

    threshold_val = value;
    ui->threshold_lineEdit->setText(QString::number(value));

    Mat dst;

    if (ui->imageMode_comboBox->currentText() != "Edge") {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, threshold_val, 255, cv::THRESH_BINARY_INV);
        updateView(dst);
    }

    if (ui->imageMode_comboBox->currentText() == "Contour") {
        Mat contourOut = contour->setImageView(src, threshold_val);
        updateView(contourOut);
    }
    else if (ui->imageMode_comboBox->currentText() == "Edge") {
        Mat edgeOut = edge->setImageView(src, threshold_val);
        updateView(edgeOut);
    }


}

void MainWindow::on_imageMode_comboBox_activated(const QString &arg1)
{
    if(!check_imageOpened()){
        errorMsg();
        ui->imageMode_comboBox->setCurrentIndex(0);
        return;
    }// error

    if (arg1 == "Original") {
        dst = imread(imagePath.toStdString());
        if (threshold_val > 0) {
            cv::threshold(src, dst, threshold_val, 255, cv::THRESH_BINARY_INV);
        }
        updateView(dst);
    }
    else if (arg1 == "Contour") {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, threshold_val, 255, cv::THRESH_BINARY_INV);
        dst = contour->setImageView(src, threshold_val);
        updateView(dst);
    }
    else if (arg1 == "Edge") {
        src = imread(imagePath.toStdString());
        dst = edge->setImageView(src, threshold_val);
        updateView(dst);
    }

}

void MainWindow::updateView(Mat imageOut)
{
    // convert Mat to QImage display on graphicsView
    if (ui->imageMode_comboBox->currentText() != "Edge") {
        cv::cvtColor(imageOut, imageOut, cv::COLOR_BGR2RGB);
        QImage img((uchar*)imageOut.data, imageOut.cols, imageOut.rows, QImage::Format_RGB888);
        image = QPixmap::fromImage(img);
    }
    else {
        QImage img((uchar*)imageOut.data, imageOut.cols, imageOut.rows, QImage::Format_Indexed8);
        image = QPixmap::fromImage(img);
    }

    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());

    ui->graphicsView->setScene(scene);

}// update graphic view

