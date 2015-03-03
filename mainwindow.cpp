#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <iostream>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->threshold_lineEdit->setText("0");
    //ui->graphicsView->installEventFilter();


    imagePtr = new Image();
    bloodVesselObject = new BloodVessels();

    threshold_val = 0;
    scaleFactor = 1.15;

    mouseEnabled = false;
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
    imagePaths = QFileDialog::getOpenFileNames(
                            this, tr("Select one or more files to open"), "",
                            tr("Images (*.png *.xpm *.jpg)"));

    if (imagePaths.isEmpty()) {
        errorMsg();
        return;
    }

    for (int i = 0; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        if (imagePath == NULL) {
            QMessageBox::about(this, tr("Error !"),
                               tr("Error loading file."));
            imagePaths.removeAt(i);
            i--;
        }
        else {
            ui->imageFiles_listWidget->insertItem(i, imagePath);
            Mat img = imread(imagePath.toStdString());
            if (i == 0) {
                src = img;
            }
            src_images.push_back(img);
        }
    }

//    imagePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "",
//                                                    tr("Images (*.png *.xpm *.jpg)"));
//    if (imagePath == NULL) {
//        errorMsg();
//        return;
//    }

//     //set source image
//    src = imread(imagePath.toStdString());
    cv::resize(src, src, cv::Size2i(src.cols/3, src.rows/3));

    //set graphic view
    imageObject = new QImage();
    imageObject->load(imagePaths.at(0));
    image = QPixmap::fromImage(*imageObject);
    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());
    ui->graphicsView->setScene(scene);

    // fit to window by default
    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->setAlignment(Qt::AlignCenter);

    ui->imageFiles_listWidget->setCurrentItem(ui->imageFiles_listWidget->item(0));

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
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if(!check_imageOpened()){
        errorMsg();
        return;
    }// error


    threshold_val = value;
    ui->threshold_lineEdit->setText(QString::number(value));

    Mat dst;

    if (ui->imageMode_comboBox->currentText() != "Edge") {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        updateView(dst);
    }

    if (ui->imageMode_comboBox->currentText() == "Contour") {
        Mat contourOut = imagePtr->setImageView(src, threshold_val, "contour");
        updateView(contourOut);
    }
    else if (ui->imageMode_comboBox->currentText() == "Edge") {
        Mat edgeOut = imagePtr->setImageView(src, threshold_val, "edge");
        updateView(edgeOut);
    }


}

void MainWindow::on_imageMode_comboBox_activated(const QString &arg1)
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if(!check_imageOpened()){
        errorMsg();
        ui->imageMode_comboBox->setCurrentIndex(0);
        return;
    }// error

    if (arg1 == "Original") {
        dst = imread(imagePath.toStdString());
        if (threshold_val > 0) {
            cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        }
        updateView(dst);
    }
    else if (arg1 == "Contour") {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        dst = imagePtr->setImageView(src, threshold_val, "contour");
        updateView(dst);
    }
    else if (arg1 == "Edge") {
        src = imread(imagePath.toStdString());
        dst = imagePtr->setImageView(src, threshold_val, "edge");
        updateView(dst);
    }
    else if (arg1 == "Skeleton") {
        src = imread(imagePath.toStdString());
        dst = imagePtr->setImageView(src, threshold_val, "skeleton");
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

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (mouseEnabled) {
        QPoint  local_pt = ui->graphicsView->mapFromGlobal(event->globalPos());
        QPointF img_coord_pt = ui->graphicsView->mapToScene(local_pt);

        qreal x_coord = img_coord_pt.x();
        qreal y_coord = img_coord_pt.y();

        bloodVesselObject->saveTipPoint(x_coord, y_coord);

        dst = bloodVesselObject->identifyTip(src, (float) x_coord, (float) y_coord);
        updateView(dst);
    }
}

void MainWindow::on_bloodVesselsTips_radioButton_toggled(bool checked)
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if (checked) {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        mouseEnabled = true;
    }
    else {
        mouseEnabled = false;
        return;
    }
}

void MainWindow::on_displayBloodVesselTips_radioButton_toggled(bool checked)
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if (checked && !bloodVesselObject->isEmpty()) {
        src = imread(imagePath.toStdString());
        dst = bloodVesselObject->displayTips(src);
        updateView(dst);
    }
}

void MainWindow::on_imageFiles_listWidget_itemClicked(QListWidgetItem *item)
{
    item->setSelected(true);
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    src = imread(imagePath.toStdString());
    if (ui->imageMode_comboBox->currentText() == "Original") {
        dst = imread(imagePath.toStdString());
        if (threshold_val > 0) {
            cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        }
        updateView(dst);
    }
    else if (ui->imageMode_comboBox->currentText() == "Contour") {
        cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        dst = imagePtr->setImageView(src, threshold_val, "contour");
        updateView(dst);
    }
    else if (ui->imageMode_comboBox->currentText() == "Edge") {
        dst = imagePtr->setImageView(src, threshold_val, "edge");
        updateView(dst);
    }
}

void MainWindow::on_displayOrigImage_pushButton_clicked()
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    src = imread(imagePath.toStdString());
    cv::resize(src, src, cv::Size2i(src.cols/4, src.rows/4));
    namedWindow(imagePath.toStdString(), WINDOW_NORMAL);
    imshow(imagePath.toStdString(), src);
}

void MainWindow::on_pushButton_3_clicked()
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    int old_thresh = threshold_val;

    if (index == 0) {
        bloodVesselObject->testTips();
        threshold_val = 178;
    }
    else {
        bloodVesselObject->testTips2();
        threshold_val = 132;
    }

    src = imread(imagePath.toStdString());
    cv::threshold(src, dst, threshold_val, 255, cv::THRESH_BINARY_INV);

//    updateView(dst);

//    Mat src_gray;
//    Mat img_bw;

//    cv::cvtColor(src, src_gray, cv::COLOR_BGR2GRAY);
//    adaptiveThreshold(src_gray, img_bw, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 53, 1);

    dilate(dst, dst, Mat(), Point(-1, -1), 3, 1, 2);
    erode(dst, dst, Mat(), Point(-1, -1), 5, 1, 2);
    dst = imagePtr->setBackground(dst);

    if (index == 1) { // doesn't seem to work for first image for some reason
        dst = imagePtr->removeOuterVessel(dst);
    }
    // some tips for testing and comparison


    bloodVesselObject->displayTips(dst);
    cv::resize(dst, dst, cv::Size2i(dst.cols/4, dst.rows/4));
    namedWindow(imagePath.toStdString(), WINDOW_NORMAL);
    imshow(imagePath.toStdString(), dst);

    threshold_val = old_thresh;

}

void MainWindow::on_tipDetect_pushButton_clicked()
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    // some tips for testing and comparison
    if (index == 0) {
        bloodVesselObject->testTips();
        threshold_val = 185;
    }
    else {
        bloodVesselObject->testTips2();
        threshold_val = 132;
    }

    src = imread(imagePath.toStdString());
    if (ui->imageMode_comboBox->currentText() == "Original") {
        dst = imread(imagePath.toStdString());
        if (threshold_val > 0) {
            cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        }
        bloodVesselObject->displayTips(dst);
        updateView(dst);
    }
    else if (ui->imageMode_comboBox->currentText() == "Contour") {
        cv::threshold(src, dst, threshold_val, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        dst = imagePtr->setImageView(src, threshold_val, "contour");
        bloodVesselObject->displayTips(dst);
        updateView(dst);
    }
    else if (ui->imageMode_comboBox->currentText() == "Edge") {
        dst = imagePtr->setImageView(src, threshold_val, "edge");
        updateView(dst);
    }
}
