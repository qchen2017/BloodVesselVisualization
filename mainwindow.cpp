#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QVector2D>
#include <QVector>
#include <QHash>
#include <QSet>
#include <QColorDialog>
#include <math.h>
#include <iostream>

#define PI 3.14159265 // to calculate angle

using namespace cv;
using namespace std;

// global variables for slide show
int trackbarNum, timeDelay, slideNum;
bool slidePause;
Mat imageRead;
QStringList pathlists;
QString pathname;
char keyPressed;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); // main window UI

    // create pointers for other objects
    imagePtr = new Image();
    bloodVesselObject = new BloodVessels(this);
    ssWin = new Slideshow(this); // has a separate UI
    edgeWin = new Edge(this);

    // initialize Help window
    helpWin = new QWidget;
    helpWin->sizeHint();
    helpWin->setWindowTitle("Help");
    helpWin->setFixedSize(200, 130);

    // use for keeping track of images that have been uploaded or closed
    imageListPtr = 0;

    // initialize variables for threshold and zoom functions
    scaleFactor = 1.15;

    // initialize color for manual tips detection - default is red
    tips_color.setRgb(255, 0, 0);
    QString qss_init = QString("background-color: %1").arg(tips_color.name());
    ui->color_pushButton->setStyleSheet(qss_init);
    ui->color_pushButton->setAutoFillBackground(true);
    ui->color_pushButton->setFlat(true);

    // control flags
    dummyImgOn = true;
    mouseEnabled = false;
    refPointEnabled = false;
    tipsEnabled = false;
    selected_ref = false;
    revert = false;
    manualSelected = false;

    ref_point.setX(0); //initialize reference point to default (0,0)
    ref_point.setY(0);


    /************************************* MAIN WINDOW UI ***************************************/

//    // setup color combo box
//    QPixmap px(15,15);
//    px.fill(QColor(Qt::red));
//    QIcon icon(px);
//    ui->color_comboBox->addItem(icon, "Red");
//    px.fill(QColor(Qt::blue));
//    icon.addPixmap(px);
//    ui->color_comboBox->addItem(icon,"Blue");
//    px.fill(QColor(Qt::green));
//    icon.addPixmap(px);
//    ui->color_comboBox->addItem(icon,"Green");

    // other initial states
    ui->threshold_lineEdit->setText("0");
    ui->actionUndo_Manual_Detect->setEnabled(false);

    // tool tips for each of the UI components
    ui->imageFiles_listWidget->setToolTip("Loaded images.");
    //ui->graphicsView->setToolTip("Current image on display.");
    ui->closeImage_toolButton->setToolTip("Closes current image on display.");

    ui->displayOrigImage_pushButton->setToolTip("Displays original image in a new window.");
    ui->imageMode_comboBox->setToolTip("Select between different image modes to display in main window.");
    ui->threshold_horizontalSlider->setToolTip("Adjusts the current image's threshold value.");
    ui->revertAllChanges_pushButton->setToolTip("Reverts the image to its original state.");
    ui->branchGraph->setToolTip("Displays graph of the blood vessel branches of all the images in a separate window.");
    ui->animate_pushButton->setToolTip("Plays the images in sequence on a separate window.");

    ui->select_ref_point_radioButton->setToolTip("Manually select reference point using mouse control.");
    ui->bloodVesselsTips_radioButton->setToolTip("Manually select tips using mouse control.");
    ui->color_pushButton->setToolTip("Select color of tip dot for manual tips detection.");
    //ui->color_comboBox->setToolTip("Select color of tip dot for manual tips detection.");
    ui->tip_size_spinBox->setToolTip("Select size of tip dot for manual tips detection.");

    ui->tipDetect_pushButton->setToolTip("Export the (x, y) coordinates of the blood vessel tips (automated) of all the images to a file.");
    ui->manual_checkBox->setToolTip("Animate manually selected tips.");
    ui->automated_checkBox->setToolTip("Animate automated tips.");
    ui->tipsAnimation_pushButton->setToolTip("Plays the tips for each image in sequence on a separate window.");

    ui->tipsXcoord_textEdit->setToolTip("Displays the X coordinates of the manually selected tips for image currently on display.");
    ui->tipsYcoord_textEdit->setToolTip("Displays the Y coordinates of the manually selected tips for image currently on display.");
    ui->angle_textEdit->setToolTip("Displays the angle between a manually selected tip and the reference point.");
    ui->length_textEdit->setToolTip("Displays the length/distance between a manually selected tip and the reference point.");

    ui->displayTips_pushButton->setToolTip("Displays the manually selected tips for image currently on display.");
    ui->exportManual_pushButton->setToolTip("Export the manually detected tips of all the images to a file.");

    // initialize help window
    static QLabel helpInfo;
    QString info("Documentations: ");
    QString designDoc("<a style=\"text-decoration: none;\" href=\"https://docs.google.com/a/ucdavis.edu/document/d/1MzFV0zI-LZV6j7tqBh1H0ese6fr5gMz8w4HmYBFGPOk/\"> Design Document</a>");
    QString testDoc("<a style=\"text-decoration: none;\" href=\"https://docs.google.com/a/ucdavis.edu/document/d/1hkfqfILpR68mZEYMrsZXgx0fQEmJTdX65G3fDLlw8MY/\"> Testing Document</a>");
    
    helpInfo.setText("<b>" + info + "</b><br><br>" + designDoc + "<br>" + testDoc);
    helpInfo.setOpenExternalLinks(true);
    helpInfo.setAlignment(Qt::AlignCenter);
 
    QVBoxLayout *vbl = new QVBoxLayout(helpWin);
    vbl->addWidget(&helpInfo);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/**********************************************************************************/
/*********************************** HELPER FUNCTIONS *****************************/
/**********************************************************************************/

void MainWindow::errorMsg()
{
    QMessageBox::about(this, tr("Error !"), tr("No loaded image. \n Please open an image first.\n"));

} // error message for no image

bool MainWindow::check_imageOpened()
{
    if (imagePath == NULL || dummyImgOn == true) {
        return false;
    }
    return true;

} // check if an image has been loaded

bool MainWindow::imageAlreadyLoaded(QString imp)
{
    QList<QListWidgetItem *>  items;
    items = ui->imageFiles_listWidget->findItems(imp, Qt::MatchExactly);
    if (!items.isEmpty()) {
        return true;
    }
    return false;
} // check if an image was already loaded before

/**********************************************************************************/
/**************************** Main Menu Bar Functions *****************************/
/**********************************************************************************/

void MainWindow::on_actionOpen_triggered()
{
    QHash<QString, QString> renames;
    QSet<QString> temp_renames;

    // ****** accepts png and jpg image types
    if (imagePaths.isEmpty()) {
        imagePaths = QFileDialog::getOpenFileNames(this, tr("Select one or more files to open"), "", tr("Images (*.png *.jpeg *.jpg)"));
    }
    else {
        imagePaths.append(QFileDialog::getOpenFileNames(this, tr("Select one or more files to open"), "", tr("Images (*.png *.jpeg *.jpg)")));

        // got through the image to determine if there are ones that have been uploaded before/have the same absolute path/name
        for (int i = imageListPtr; i < imagePaths.size(); i++) {

            // ***** image was already uploaded before?
            if (imageAlreadyLoaded(imagePaths.at(i))) {
                QString alrUploaded = imagePaths.at(i);
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Image", alrUploaded.append(" already exists. Would you like to rename this image?"), QMessageBox::Yes|QMessageBox::No);

                // **** user wants to rename it?
                if (reply == QMessageBox::Yes) {
                    bool ok;
                    QString text = QInputDialog::getText(this, tr("Image"), tr("New Name:"), QLineEdit::Normal, QDir::home().dirName(), &ok);

                    // *** if a name was entered
                    if (ok && !text.isEmpty()) {

                        // ** check that it's a valid name/name hasn't been used before
                        if ((ui->imageFiles_listWidget->findItems(text, Qt::MatchExactly)).isEmpty()) {

                            // * check if name hasn't been used as a rename before
                            if (temp_renames.find(text) == temp_renames.end()) {
                                renames[imagePaths.at(i)] = text;
                                temp_renames.insert(text);
                            }
                            else {
                                QString alertMsg = text;
                                QMessageBox::information(this, "Image", alertMsg.append(" is already in use. The image will not be uploaded."));
                                imagePaths.removeAt(i);
                                i--;
                            } // end if else *
                        }
                        else {
                            QString alertMsg = text;
                            QMessageBox::information(this, "Image", alertMsg.append(" is already in use. The image will not be uploaded."));
                            imagePaths.removeAt(i);
                            i--;
                        } // end if else **
                    }
                    else {
                        QMessageBox::information(this, "Image", "Image will not be renamed.");
                        imagePaths.removeAt(i);
                        i--;
                    } // end if else ***

                }
                else {
                    imagePaths.removeAt(i);
                    i--;
                } // end if else ****
            } // end if ******
        } // end for loop
    } // end if ******

    // checks if image(s) exist
    if (imagePaths.isEmpty()) {
        errorMsg();
        return;
    }

    // store all the absolute paths of all the images that are being loaded
    for (int i = imageListPtr; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);

        // alert user if there's an error with one or more of the images
        if (imagePath == NULL) { 
            QMessageBox::about(this, tr("Error!"), tr("Error loading file."));
            imagePaths.removeAt(i);
            i--;
        }
        else {
            // check if image hasn't been loaded before
            if (!imageAlreadyLoaded(imagePath)) {
                ui->imageFiles_listWidget->insertItem(i, imagePath);
                Mat img = imread(imagePath.toStdString());
                if (i == 0) {
                    src = img; // set current src image
                }
                src_images.push_back(img);
                thresholds[imagePath.toStdString()] = 0;
            }            
            else {
                // check if the name of the image hasn't been used as a rename
                if (renames.find(imagePath) != renames.end()) {
                    ui->imageFiles_listWidget->insertItem(i, renames[imagePath]);
                    Mat img = imread(imagePath.toStdString());
                    if (i == 0) {
                        src = img; // set current src image
                    }
                    src_images.push_back(img);
                    thresholds[imagePath.toStdString()] = 0;
                }
            }
        }
    }

    // keep track of current number of images
    imageListPtr = imagePaths.size();

    // assign an image to src_resize
    cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));

    // set graphic view (main application window)
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

    // set highlighted item in image files list to be the first item on the list
    ui->imageFiles_listWidget->setCurrentItem(ui->imageFiles_listWidget->item(0));

    dummyImgOn = false;

    // enable menu bar and UI functions
    ui->actionSave->setEnabled(true);
    ui->actionFit_to_Window->setEnabled(true);
    ui->actionZoom_In_->setEnabled(true);
    ui->actionZoom_Out->setEnabled(true);

    ui->threshold_horizontalSlider->setEnabled(true);
    ui->imageMode_comboBox->setEnabled(true);
    ui->displayOrigImage_pushButton->setEnabled(true);
    ui->branchGraph->setEnabled(true);
    ui->animate_pushButton->setEnabled(true);
    ui->revertAllChanges_pushButton->setEnabled(true);
    ui->closeImage_toolButton->setEnabled(true);

    ui->bloodVesselsTips_radioButton->setEnabled(true);
    ui->select_ref_point_radioButton->setEnabled(true);
    ui->color_pushButton->setEnabled(true);
    //ui->color_comboBox->setEnabled(true);
    ui->tip_size_spinBox->setEnabled(true);

    ui->manual_checkBox->setEnabled(true);
    ui->automated_checkBox->setEnabled(true);
    ui->tip_checkBox->setEnabled(true);
    
    ui->tipsAnimation_pushButton->setEnabled(true);
    ui->exportManual_pushButton->setEnabled(true);
    ui->displayTips_pushButton->setEnabled(true);
    ui->clearTips_pushButton->setEnabled(true);
    ui->tipDetect_pushButton->setEnabled(true);

} // open image

void MainWindow::on_actionSave_triggered()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    // save image to either jpg or png formats
    imagePath = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                                                     tr("JPEG (*.jpg *.jpeg);;PNG (*.png)"));
    *imageObject = image.toImage();
    imageObject->save(imagePath);

} // save image

void MainWindow::on_actionFit_to_Window_triggered()
{
    if (scene == NULL || dummyImgOn == true) {
        errorMsg();
        return;
    }

    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
} // fit to window

void MainWindow::on_actionZoom_In__triggered()
{
    if (scene == NULL || dummyImgOn == true) {
        errorMsg();
        return;
    } // error

    ui->graphicsView->scale(scaleFactor, scaleFactor);
} // zoom in

void MainWindow::on_actionZoom_Out_triggered()
{
    if (scene == NULL || dummyImgOn == true) {
        errorMsg();
        return;
    } // error

    ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
} // zoom out

void MainWindow::on_actionUndo_Manual_Detect_triggered()
{
    // if there is an image
    if (imageListPtr > 0) {
        int index = ui->imageFiles_listWidget->currentRow();
        imagePath = imagePaths.at(index);
        string imp = imagePath.toStdString();
        src = imread(imp);

        // if this image has manually detected tips saved
        if (!bloodVesselObject->thisImageTipsIsEmpty(imp)) {
            bloodVesselObject->deleteTipPoint(imp);

            // update the text boxes to reflect undoing
            QStringList xbox_lines = (ui->tipsXcoord_textEdit->toPlainText()).split("\n");
            xbox_lines.removeLast();
            ui->tipsXcoord_textEdit->setPlainText(xbox_lines.join("\n"));

            QStringList ybox_lines = (ui->tipsYcoord_textEdit->toPlainText()).split("\n");
            ybox_lines.removeLast();
            ui->tipsYcoord_textEdit->setPlainText(ybox_lines.join("\n"));

            QStringList length_lines = (ui->length_textEdit->toPlainText()).split("\n");
            length_lines.removeLast();
            ui->length_textEdit->setPlainText(length_lines.join("\n"));

            QStringList angle_lines = (ui->angle_textEdit->toPlainText()).split("\n");
            angle_lines.removeLast();
            ui->angle_textEdit->setPlainText(angle_lines.join("\n"));

            // check again if there are more saved tips for this image - disable menu bar action if all tips have been undone
            if (bloodVesselObject->thisImageTipsIsEmpty(imp)) {
                ui->actionUndo_Manual_Detect->setDisabled(true);
            }

            // update current image ond display to reflect undoing
            int tips_size = ui->tip_size_spinBox->value();
            dst = bloodVesselObject->displayTips(src, imagePath.toStdString(), tips_size, tips_color);
            updateView(dst);
        }
    }
}

void MainWindow::on_actionReset_triggered()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    tips_map.erase(imagePath.toStdString());
    bloodVesselObject->deleteAllTipPoints(imagePath.toStdString());
    src = imread(imagePath.toStdString());

    ui->exportManual_pushButton->setDisabled(true);
    ui->tipsXcoord_textEdit->clear();
    ui->tipsYcoord_textEdit->clear();
    ui->length_textEdit->clear();
    ui->angle_textEdit->clear();
    ui->actionReset->setDisabled(true);

    updateView(src);

}

void MainWindow::on_actionView_Documentation_triggered()
{
    helpWin->show();
}

/**********************************************************************************/
/*********************** Main User Interface Functionalities **********************/
/**********************************************************************************/

void MainWindow::close_opencv_window(string window_name)
{
    while (true) {
        char c = waitKey(33);
        if (c == 27) {
            destroyWindow(window_name);
        }
    } // while not esc
} // closes an open cv window

void MainWindow::on_revertAllChanges_pushButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    src = imread(imagePath.toStdString());
    thresholds[imagePath.toStdString()] = 0;
    ui->threshold_horizontalSlider->setValue(0);
    ui->imageMode_comboBox->setCurrentIndex(0);
    updateView(src);

}

void MainWindow::on_closeImage_toolButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = 0;
    
    // if there are more than one images currently loaded, display either the image before or after the one that was closed
    if (imagePaths.size() > 1) {
        index = ui->imageFiles_listWidget->currentRow();

        // if it is the first image display the one after it
        if (index == 0) {
            imagePath = imagePaths.at(index + 1);
        }

        else {
            imagePath = imagePaths.at(index - 1);
        }

        src = imread(imagePath.toStdString());
        updateView(src);

    }
    else { // else, there was only one image, so put a dummy image after the only image is closed
        dummyImgOn = true;
        Mat dummyImg = src_resize;
        dummyImg.setTo(Scalar(255, 255, 255));
        updateView(dummyImg);
    }

    // update the containers and other variables related to image files handling
    if (imageListPtr > 0) {
        imageListPtr--;
        src_images.remove(index);
        imagePaths.removeAt(index);
        tips_map.erase(imagePath.toStdString());
        thresholds.erase(imagePath.toStdString());
        ui->imageFiles_listWidget->takeItem(index);
    }

} // close current image on display

void MainWindow::on_imageFiles_listWidget_itemClicked(QListWidgetItem *item)
{
    item->setSelected(true); // the selected image from the list
    int index = ui->imageFiles_listWidget->currentRow(); // index of the selected image
    imagePath = imagePaths.at(index); // absolute path of the selected image
    src = imread(imagePath.toStdString());
    int t = thresholds[imagePath.toStdString()];
    ui->threshold_horizontalSlider->setValue(t);
    ui->tipsXcoord_textEdit->clear();
    ui->tipsYcoord_textEdit->clear();
    ui->length_textEdit->clear();
    ui->angle_textEdit->clear();

    // display the appropriate image in the main window
    if (ui->imageMode_comboBox->currentText() == "Normal") {
        dst = imread(imagePath.toStdString());
        if (t != 0) {
            cv::threshold(src, dst, t, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        }
        updateView(dst);
    }
    else if (ui->imageMode_comboBox->currentText() == "Contour") {
        cv::threshold(src, dst, t, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        dst = imagePtr->setImageView(src, t, "contour");
        updateView(dst);
    }
    else if (ui->imageMode_comboBox->currentText() == "Edge") {
        src = imread(imagePath.toStdString());
        dst = edgeWin->setEdge(src, t);
        updateView(dst);
    }

}


void MainWindow::on_displayOrigImage_pushButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    src = imread(imagePath.toStdString());
    if (src.cols > 1500 || src.rows > 1500) { // if image is to big, resize first
        cv::resize(src, src, cv::Size2i(src.cols/3, src.rows/3));
    }
    namedWindow(imagePath.toStdString(), WINDOW_NORMAL);
    imshow(imagePath.toStdString(), src);
    close_opencv_window(imagePath.toStdString());
}

void MainWindow::on_threshold_horizontalSlider_valueChanged(int value)
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    ui->threshold_lineEdit->setText(QString::number(value));

    thresholds[imagePath.toStdString()] = value; // map threshold to corresponding image

    // update view depending on mode
    Mat img;
    if (ui->imageMode_comboBox->currentText() == "Contour") {
        contourOut = imagePtr->setImageView(src, value, "contour");
        updateView(contourOut);
    }
    else if (ui->imageMode_comboBox->currentText() == "Edge") {
        dst = edgeWin->setEdge(src, value);
        updateView(dst);
    }
    else {
        src = imread(imagePath.toStdString());
        cv::threshold(src, img, value, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        updateView(img);
    }
}

void MainWindow::on_imageMode_comboBox_activated(const QString &arg1)
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    // get current image
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    int t = thresholds[imagePath.toStdString()];

    // display appropriate mode
    if (arg1 == "Normal") {
        dst = imread(imagePath.toStdString());
        if (t > -1) {
            cv::threshold(src, dst, t, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        }
        updateView(dst);
    }
    else if (arg1 == "Contour") {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, t, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        dst = imagePtr->setImageView(src, t, "contour");
        updateView(dst);
    }
    else if (arg1 == "Edge") {
        src = imread(imagePath.toStdString());
        dst = edgeWin->setEdge(src, t);
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

    // update main view/graphics view
    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());

    ui->graphicsView->setScene(scene);

} // update graphic view

/**********************************************************************************/
/********************* Functions for Manual Tips Detection ************************/
/**********************************************************************************/
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (check_imageOpened()) {

        int index = ui->imageFiles_listWidget->currentRow();
        imagePath = imagePaths.at(index);
        ui->exportManual_pushButton->setEnabled(true);

        if (mouseEnabled) {
            QPoint  local_pt = ui->graphicsView->mapFromGlobal(event->globalPos());
            QPointF img_coord_pt = ui->graphicsView->mapToScene(local_pt);

            qreal x_coord = img_coord_pt.x();
            qreal y_coord = img_coord_pt.y();
            bloodVesselObject->saveTipPoint(imagePath.toStdString(), x_coord, y_coord);

            // adjusted based on reference point
            qreal selected_x = (qreal)(img_coord_pt.x() - src.cols/2)/(qreal)(src.cols/2);
            qreal selected_y = (qreal)(src.rows/2 - img_coord_pt.y())/(qreal)(src.rows/2);
            qreal x2_x1 = selected_x - ref_point.x();
            qreal y2_y1 = selected_y - ref_point.y();

            qreal length = 0, angle = 0;
            length = sqrt(x2_x1 * x2_x1 + y2_y1 * y2_y1);

            //atan2 returns positive angle for positive Y position, and vice versa
            angle = atan2(y2_y1, x2_x1) * 180 / PI;
            if (angle < 0) { //recalculate to get a positive angle value
                angle = 360 + angle;
            }

            // each (x, y) point is displayed in their appropriate text edits
            QString x = QString::number((double) x2_x1, 'g', 3);
            QString y = QString::number((double) y2_y1, 'g', 3);
            QString l = QString::number((double)length, 'g', 3); //length
            QString a = QString::number((double)angle, 'g', 3); //angle

            //disregard clicks that were pressed outside of the image
            if (selected_x >= -1 && selected_x <= 1 && selected_y >= -1 && selected_y <= 1) {
                ui->actionUndo_Manual_Detect->setEnabled(true);
                ui->tipsXcoord_textEdit->append(x);
                ui->tipsYcoord_textEdit->append(y);
                ui->length_textEdit->append(l);
                ui->angle_textEdit->append(a);

            }
            // display the tips in real time
            int tips_size = ui->tip_size_spinBox->value();
            dst = bloodVesselObject->identifyTip(src, (float) x_coord, (float) y_coord, tips_size, tips_color);
            updateView(dst);
        }
        else if (refPointEnabled && !mouseEnabled) {

            QPoint  local_pt = ui->graphicsView->mapFromGlobal(event->globalPos());
            QPointF img_coord_pt = ui->graphicsView->mapToScene(local_pt);

            // adjusted based on reference point
            qreal adjusted_x = (qreal)(img_coord_pt.x() - src.cols/2)/(qreal)(src.cols/2);
            qreal adjusted_y = (qreal)(src.rows/2 - img_coord_pt.y())/(qreal)(src.rows/2);

            // each (x, y) point is displayed in their appropriate text edits
            // (0, 0) is at the center of the image
            QString x = QString::number((double) adjusted_x, 'g', 3);
            QString y = QString::number((double) adjusted_y, 'g', 3);
            QString ref = "Keep (" + x + ", " + y + ") as reference point?";

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Reference Point", ref,
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                ref_point.setX(adjusted_x);
                ref_point.setY(adjusted_y);
                QString rx = QString::number((double)ref_point.x(), 'g', 3);
                QString ry = QString::number((double)ref_point.y(), 'g', 3);

                QString r = rx + ", " + ry;
                ui->refpoint_lineEdit->setText(r);
                refPointEnabled = false;
                selected_ref = true;
                if (revert) {
                    mouseEnabled = true;
                    revert = false;
                }
                ui->tipsXcoord_textEdit->clear();
                ui->tipsYcoord_textEdit->clear();
                ui->length_textEdit->clear();
                ui->angle_textEdit->clear();
            }
        }

    }

}

void MainWindow::on_bloodVesselsTips_radioButton_toggled(bool checked)
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    int t = thresholds[imagePath.toStdString()];

    if (checked) {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, t, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        mouseEnabled = true;
        refPointEnabled = false;
    }
    else {
        mouseEnabled = false;
        return;
    }

    ui->actionReset->setEnabled(true);

} // enable manual detection of tips

void MainWindow::on_color_pushButton_clicked()
{
    QColorDialog *qcd = new QColorDialog(QColor::fromRgb(0, 0, 0, 255), this);
    QColor color = qcd->getColor();
    if(color.isValid()) {
        QString qss = QString("background-color: %1").arg(color.name());
        ui->color_pushButton->setStyleSheet(qss);
        ui->color_pushButton->setAutoFillBackground(true);
        ui->color_pushButton->setFlat(true);
        tips_color = color;
    }
}


void MainWindow::on_select_ref_point_radioButton_clicked()
{

    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    refPointEnabled = true;
    
    if (mouseEnabled) {
        mouseEnabled = false;
        revert = true;
    }

} // select reference point

void MainWindow::on_tip_checkBox_clicked(bool checked)
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    if (checked)
        tipsEnabled = true;
    else
        tipsEnabled = false;
} // include tips

void MainWindow::on_displayTips_pushButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if (!bloodVesselObject->isEmpty()) {
        ui->actionUndo_Manual_Detect->setEnabled(true);
        src = imread(imagePath.toStdString());
        int tips_size = ui->tip_size_spinBox->value();
        dst = bloodVesselObject->displayTips(src, imagePath.toStdString(), tips_size, tips_color);
        updateView(dst);
    }

}


void MainWindow::on_clearTips_pushButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if (!bloodVesselObject->isEmpty()) {
        src = imread(imagePath.toStdString());
        updateView(src);
    }
}

/**********************************************************************************/
/************************ Main Application Functionalities ************************/
/**********************************************************************************/

void MainWindow::on_tipDetect_pushButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    // prompt user to manually select threshold value for each image or let program do it
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Threshold Value", "Would you like to use the current threshold value for all of the images?",
                                  QMessageBox::Yes|QMessageBox::No);


    if (reply == QMessageBox::Yes) { // currently set threshold value for each image
        // notify user
        QMessageBox::information(
                this,
                tr("Visualization of Directional Blood Vessels"),
                tr("The application will use the currently set threshold value for each image. "
                   "If you would like to set different threshold values, click 'Cancel' on the next window to go back.") );

        // loop through each image and detect tips for each
        for (int i = 0; i < imagePaths.size(); i++) {
            imagePath = imagePaths.at(i);
            src = imread(imagePath.toStdString());
            cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));
            string imName = imagePath.toStdString();
            edgeWin->detectTips(src_resize, tips_map, imName, thresholds[imName]);
        }

        writeTipsToFile(tips_map);
    }
    else if (reply == QMessageBox::No) {
        for (int i = 0; i < imagePaths.size(); i++) {
            imagePath = imagePaths.at(i);
            src = imread(imagePath.toStdString());
            cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));

            // detect tips; 135 is the threshold value used by the program
            edgeWin->detectTips(src_resize, tips_map, imagePath.toStdString(), 135);

            // prints out vector of coordinates for debugging purposes
            // QVector<QVector2D> pts;
            // pts = tips_map[imagePath.toStdString()];
            // qDebug() << pts;
        }

        writeTipsToFile(tips_map);
    }

    // if the prompt was closed instead of responding YES or NO, nothing will happen

} // automated tip detection

void MainWindow::writeTipsToFile(unordered_map<string, QVector<QVector2D> > tips_map)
{
    qreal x2_x1;
    qreal y2_y1;

    // prompt user for file name and location
    //QString outfile = QFileDialog::getSaveFileName(this, "Save");
    QString outfile = QFileDialog::getSaveFileName(this, "Save", "untitled.csv", tr("Comma Separated Values (CSV) (*.csv);;"
                                                                                       "Excel Workbook (*.xlsx);; "
                                                                                       "Excel 97-2003 Workbook (*.xls);;"
                                                                                       "Text (*.txt);;"
                                                                                       "OpenDocument Spreadsheet (*.ods)"));

    // if a file name is selected/specified, write tips coordinates to file
    // otherwise don't do anything
    if (outfile != NULL) {
        QFile file(outfile);
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream stream(&file);

            // iterate through tips_map to get the tips' coordinates for each image
            for(auto it = tips_map.begin(); it != tips_map.end(); ++it) {
                string temp = it->first; // image path name

                QString imgname = QString::fromStdString(temp);
                stream << imgname << endl; // write image path name

                //set up column names
                stream << "X,Y";
                stream << ",Length";
                stream << ",Angle";
                stream << endl;

                QVector<QVector2D> pts = tips_map[temp]; // coordinates associated with image
                for (int i = 0; i < pts.size(); i++) {
                    QVector2D pt = pts.at(i);

                    //in automatic detection, below two values will already be in range [-1,1]
                    x2_x1 = pt.x() - ref_point.x();
                    y2_y1 = pt.y() - ref_point.y();

                    //if manual selection is true, must convert tip points into range [-1,1]
                    if (manualSelected == true) {
                        qreal selected_x = (qreal)(pt.x() - src.cols/2)/(qreal)(src.cols/2);
                        qreal selected_y = (qreal)(src.rows/2 - pt.y())/(qreal)(src.rows/2);
                        x2_x1 = selected_x - ref_point.x();
                        y2_y1 = selected_y - ref_point.y();
                    }

                    stream << x2_x1 << "," << y2_y1; // write all X, Y coordinates to file

                    qreal length = 0.0, angle = 0.0;
                    length = sqrt(x2_x1 * x2_x1 + y2_y1 * y2_y1);
                    stream << "," << length; //append length vaue to current row

                    angle = atan2(y2_y1, x2_x1) * 180 / PI;
                    if (angle < 0) {
                        angle = 360 + angle;
                    }
                    stream << "," << angle; //append angle value to current row
                    stream << endl;
                }
            }
        } // if file.open()
        file.close();
     }

    manualSelected = false;

} // write tips to file

void MainWindow::on_exportManual_pushButton_clicked() {

    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    manualSelected = true;
    unordered_map<string, QVector<QVector2D> > tips_map;
    bloodVesselObject->getManuallySelectedTips(tips_map);
    writeTipsToFile(tips_map);
} // export the manually detected tips

void MainWindow::on_branchGraph_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    QVector<QVector2D> graph_pts; // vector for the pts to be included in the graph

    Mat imageOut;
    for (int i = 0; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        src = imread(imagePath.toStdString());
        cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));
        edgeWin->branchGraph(src_resize, imageOut); // call function to generate branch graph

        // store the pixels to be included in the graph
        QVector2D pt;
        // qDebug() << imageOut.cols << imageOut.rows;
        for (int x = 300; x < (imageOut.cols - 300); x++) {
            for (int y = 0; y < imageOut.rows; y++) {
                Vec3b color = imageOut.at<Vec3b>(Point(x,y));
                if ((color[0] != 0) || (color[1] != 0) || (color[2] != 0)) {
                    //qDebug() << color[0] << color[1] << color[2];
                    pt.setX(x);
                    pt.setY(y);
                    graph_pts.push_back(pt);
                }
            }
        }
    }

    // create a new Mat for the graph
    Mat img = imageOut;
    img.setTo(cv::Scalar(0, 0, 0)); // fill image with black

    Vec3b color;
    color[0] = 255; color[1] = 255; color[2] = 255;
    QVector2D pt2;
    int x, y;

    // add each pixel (white) stored in graph_pts to the graph image
    for (int i = 0; i < graph_pts.size(); i++) {
        pt2 = graph_pts.at(i);
        x = pt2.x();
        y = pt2.y();
        img.at<Vec3b>(Point(x,y)) = color;
    }

    imshow("Graph", img);
    close_opencv_window("Graph");

} // branch graph

/**********************************************************************************/
/****************************** Slideshow Functions *******************************/
/**********************************************************************************/

void MainWindow::on_animate_pushButton_clicked()
{

    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    if (imagePaths.size() == 1) {
        QMessageBox::information(this, tr("Visualization of Directional Blood Vessels"),
                tr("There is only one image. Please upload one or more images to play a sequence."));
    }
    else {
        ssWin->setImageList(imagePaths, false);
        ssWin->show();
    }

}

void MainWindow::promptForTipsAnimation(unordered_map<string, QVector<QVector2D> > &tips_map_temp)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Threshold Value", "Would you like to use the current threshold value for all of the images?", QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, tr("Visualization of Directional Blood Vessels"), tr("The application will use the currently set threshold value for each image.") );
        findAllTips(false, tips_map_temp);
    }
    else if (reply == QMessageBox::No) {
        findAllTips(true, tips_map_temp);
    }
}

void MainWindow::findAllTips(bool threshold_default, unordered_map<string, QVector<QVector2D> > &tips_map_temp)
{
    //cout << imagePaths.size() << endl;
    for (int i = 0; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        src = imread(imagePath.toStdString());
        cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));
        string imName = imagePath.toStdString();
        if (!threshold_default) {
            edgeWin->detectTips(src_resize, tips_map_temp, imName, thresholds[imName]);
        }
        else {
            edgeWin->detectTips(src_resize, tips_map_temp, imName, 135);
        }
        edgeWin->convertToPixelCoords(src_resize, tips_map_temp, imName);
    }

    /* DEBUGGING
    for (auto it = tips_map_temp.begin(); it != tips_map_temp.end(); it++) {
        string temp = it->first;
        QVector<QVector2D> pts = tips_map_temp[temp];
        cout << temp << endl;
        for (int i = 0; i < pts.size(); i++) {
            QVector2D pt = pts.at(i);
            cout << "Converted: " << pt.x() << " " << pt.y() << endl;
        }
    }
    */
}

void MainWindow::automatedTipsAnimation(QVector<Mat> &auto_tips_images, unordered_map<string, QVector<QVector2D> > &tips_map_temp)
{
    bool threshold_default = true;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Threshold Value", "Would you like to use the current threshold value for all of the images?", QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, tr("Visualization of Directional Blood Vessels"), tr("The application will use the currently set threshold value for each image.") );
        threshold_default = false;
    }
    else if (reply == QMessageBox::No) {
        threshold_default = true;
    }

    for (int i = 0; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        src = imread(imagePath.toStdString());
        cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));
        string imName = imagePath.toStdString();
        if (!threshold_default) {
            edgeWin->detectTips(src_resize, tips_map_temp, imName, thresholds[imName]);
        }
        else {
            edgeWin->detectTips(src_resize, tips_map_temp, imName, 135);
        }
    }

    edgeWin->getAutoTipsImages(auto_tips_images);
}

void MainWindow::on_tipsAnimation_pushButton_clicked()
{
    if (!check_imageOpened()) {
        errorMsg();
        return;
    } // error

    unordered_map<string, QVector<QVector2D> > tips_map_temp;

    // both are unchecked
    if (!ui->automated_checkBox->isChecked() && !ui->manual_checkBox->isChecked()) {
        QMessageBox::about(this, tr("Error !"), tr("Please make a selection on which tips to animate."));
    }

    // only automated is checked
    else if (ui->automated_checkBox->isChecked() && !ui->manual_checkBox->isChecked())  {
        QVector<Mat> automatedTipsMats;
        automatedTipsAnimation(automatedTipsMats, tips_map_temp);
        ssWin->tipsSlideshow(automatedTipsMats, true);
        ssWin->setImageList(imagePaths, true);
        ssWin->show();
    }
    // only manual is checked
    else if (ui->manual_checkBox->isChecked() && !ui->automated_checkBox->isChecked()) {
        if (!bloodVesselObject->isEmpty()) {
            bloodVesselObject->getManuallySelectedTips(tips_map_temp);
            bloodVesselObject->tipsAnimation(tips_map_temp);
            QVector<Mat> ims = bloodVesselObject->getTipsImages();
            ssWin->tipsSlideshow(ims, false);
            ssWin->setImageList(imagePaths, true);
            ssWin->show();
        }
        else {
            QMessageBox::information(this, tr("Visualization of Directional Blood Vessels"), tr("No points have been selected. Nothing to show here.") );
        }
    }
    // both are checked
    else {
        QVector<Mat> automatedTipsMats;
        automatedTipsAnimation(automatedTipsMats, tips_map_temp);
        if (!bloodVesselObject->isEmpty()) {
            bloodVesselObject->getManuallySelectedTips(tips_map_temp);
            for (int i = 0; i < imagePaths.size(); i++) {
                imagePath = imagePaths.at(i);
                string imp = imagePath.toStdString();
                QVector<QVector2D> pts = tips_map_temp[imp];
                Mat newImg = automatedTipsMats.at(i);
                for (int j = 0; j < pts.size(); j++) {
                    QVector2D pt = pts.at(j);
                    int x = pt.x()/3;
                    int y = pt.y()/3;
                    Point dot = Point(x, y);
                    circle(newImg, dot, 5.0, Scalar(255, 255, 255), -1, 8);
                }
                automatedTipsMats.replace(i, newImg);
            }
        }
        ssWin->tipsSlideshow(automatedTipsMats, true);
        ssWin->setImageList(imagePaths, true);
        ssWin->show();
    }
} // tips animation

