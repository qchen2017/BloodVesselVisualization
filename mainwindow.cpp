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
#include <math.h>
#include <iostream>

#define PI 3.14159265 //to calculate angle

using namespace cv;
using namespace std;

//global variables for slide show
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

    ssWin = new Slideshow(this);
    edgeWin = new Edge(this); // separate UI for Edge mode
    helpWin = new QWidget; // help window
    helpWin->sizeHint();
    helpWin->setWindowTitle("Help");
    helpWin->setFixedSize(200, 130);
    ui->setupUi(this); // main window UI
    ui->threshold_lineEdit->setText("0"); // initialize line edit for the threshold value
    ui->actionUndo_Manual_Detect->setEnabled(false);

    //setup color combo box
    QPixmap px(15,15);
    px.fill(QColor(Qt::red));
    QIcon icon(px);
    ui->color_comboBox->addItem(icon, "Red");
    px.fill(QColor(Qt::blue));
    icon.addPixmap(px);
    ui->color_comboBox->addItem(icon,"Blue");
    px.fill(QColor(Qt::green));
    icon.addPixmap(px);
    ui->color_comboBox->addItem(icon,"Green");

    // create pointers for image and blood vessel objects
    imagePtr = new Image();
    bloodVesselObject = new BloodVessels(this); // also has a separate UI

    // initialize variables for threshold and zoom functions
    scaleFactor = 1.15;

    // use for manual detection of blood vessel tips
    mouseEnabled = false;

    imageListPtr = 0;

    // intialization for selecting reference point & length
    refPointEnabled = false;
    lengthEnabled = false;
    tipsEnabled = false;
    angleEnabled = false;
    selected_ref = false;
    revert = false;
    manualSelected = false;

    ref_point.setX(0); //initialize reference point to default (0,0)
    ref_point.setY(0);

    // tool tips for each of the UI components
    ui->displayOrigImage_pushButton->setToolTip("Displays original image in a new window.");
    ui->imageMode_comboBox->setToolTip("Select between different image modes to display in main window.");
    ui->threshold_horizontalSlider->setToolTip("Adjusts the current image's threshold value.");
    ui->tipDetect_pushButton->setToolTip("Writes the (x, y) coordinates of the blood vessel tips of all the images to a file.");
    ui->branchGraph->setToolTip("Displays graph of the blood vessel branches of all the images in a separate window.");
    ui->edgeButton->setToolTip("Displays Edge mode in a different window.");
    ui->animate_pushButton->setToolTip("Plays the images in sequence on a separate window.");
    ui->imageFiles_listWidget->setToolTip("Loaded images");
    ui->graphicsView->setToolTip("Current image");
    ui->closeImage_toolButton->setToolTip("Closes current image on display.");
    ui->bloodVesselsTips_radioButton->setToolTip("Manually select tips using mouse control.");
    ui->select_ref_point_radioButton->setToolTip("Manually select reference point using mouse control.");
    ui->tipsXcoord_textEdit->setToolTip("Displays the X coordinates of the manually selected tips for image currently on display.");
    ui->tipsYcoord_textEdit->setToolTip("Displays the Y coordinates of the manually selected tips for image currently on display.");
    ui->displayTips_pushButton->setToolTip("Displays the manually selected tips for image currently on display.");
    ui->tipsAnimation_pushButton->setToolTip("Plays the tips for each image in sequence on a separate window.");
    ui->manual_checkBox->setToolTip("Animate manually selected tips.");
    ui->automated_checkBox->setToolTip("Animate automated tips.");

    static QLabel helpInfo;
    QString info("Documentations: ");
    QString designDoc("<a style=\"text-decoration: none;\" href=\"https://docs.google.com/a/ucdavis.edu/document/d/1MzFV0zI-LZV6j7tqBh1H0ese6fr5gMz8w4HmYBFGPOk/\"> Design Document</a>");
    QString testDoc("<a style=\"text-decoration: none;\" href=\"https://docs.google.com/a/ucdavis.edu/document/d/1hkfqfILpR68mZEYMrsZXgx0fQEmJTdX65G3fDLlw8MY/\"> Testing Document</a>");
    
    helpInfo.setText("<b>" + info + "</b><br><br>" + designDoc + "<br>" + testDoc);
    helpInfo.setOpenExternalLinks(true);
    helpInfo.setAlignment(Qt::AlignCenter);
    
    //helpInfo.setText("Design Document: https://docs.google.com/a/ucdavis.edu/document/d/1MzFV0zI-LZV6j7tqBh1H0ese6fr5gMz8w4HmYBFGPOk/\n\n"
    //                 "Testing Document: https://docs.google.com/a/ucdavis.edu/document/d/1hkfqfILpR68mZEYMrsZXgx0fQEmJTdX65G3fDLlw8MY/");
    
    QVBoxLayout *vbl = new QVBoxLayout(helpWin);
    vbl->addWidget(&helpInfo);

}

MainWindow::~MainWindow()
{

    delete ui;
}


/**********************************************************************************/
/**************************** Main Menu Bar Functions *****************************/
/**********************************************************************************/

void MainWindow::errorMsg()
{
    QMessageBox::about(this, tr("Error !"), tr("No loaded image. \n Please open an image first.\n"));

} // error message for no image

bool MainWindow::check_imageOpened()
{
    if (imagePath == NULL) {
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
}

void MainWindow::on_actionOpen_triggered()
{
    QHash<QString, QString> renames;
    QSet<QString> temp_renames;

    // accepts png and jpg image types
    if (imagePaths.isEmpty()) {
        imagePaths = QFileDialog::getOpenFileNames(this, tr("Select one or more files to open"), "", tr("Images (*.png *.jpeg *.jpg)"));
    }
    else {
        imagePaths.append(QFileDialog::getOpenFileNames(this, tr("Select one or more files to open"), "", tr("Images (*.png *.jpeg *.jpg)")));

        // got through the image to determine if there are ones that have been uploaded before/have the same absolute path/name
        for (int i = imageListPtr; i < imagePaths.size(); i++) {

            // image was already uploaded before?
            if (imageAlreadyLoaded(imagePaths.at(i))) {
                QString alrUploaded = imagePaths.at(i);
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Image", alrUploaded.append(" already exists. Would you like to rename this image?"), QMessageBox::Yes|QMessageBox::No);

                // user wants to rename it?
                if (reply == QMessageBox::Yes) {
                    bool ok;
                    QString text = QInputDialog::getText(this, tr("Image"), tr("New Name:"), QLineEdit::Normal, QDir::home().dirName(), &ok);

                    // if a name was entered
                    if (ok && !text.isEmpty()) {

                        // check that it's a valid name/name hasn't been used before
                        if ((ui->imageFiles_listWidget->findItems(text, Qt::MatchExactly)).isEmpty()) {

                            // check if name hasn't been used as a rename before
                            if (temp_renames.find(text) == temp_renames.end()) {
                                renames[imagePaths.at(i)] = text;
                                temp_renames.insert(text);
                            }
                            else {
                                QString alertMsg = text;
                                QMessageBox::information(this, "Image", alertMsg.append(" is already in use. The image will not be uploaded."));
                                imagePaths.removeAt(i);
                                i--;
                            }
                        }
                        else {
                            QString alertMsg = text;
                            QMessageBox::information(this, "Image", alertMsg.append(" is already in use. The image will not be uploaded."));
                            imagePaths.removeAt(i);
                            i--;
                        }
                    }
                    else {
                        QMessageBox::information(this, "Image", "Image will not be renamed.");
                        imagePaths.removeAt(i);
                        i--;
                    }

                }
                else {
                    imagePaths.removeAt(i);
                    i--;
                }
            }
        }
    }

    // checks if image(s) exist
    if (imagePaths.isEmpty()) {
        errorMsg();
        return;
    }

    // store all the absolute paths of all the images that are being loaded
    for (int i = imageListPtr; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        if (imagePath == NULL) { // alert user if there's an error with one or more of the images
            QMessageBox::about(this, tr("Error!"), tr("Error loading file."));
            imagePaths.removeAt(i);
            i--;
        }
        else { // insert image into image files list
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

    imageListPtr = imagePaths.size();

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

    ui->actionSave->setEnabled(true);
    ui->actionFit_to_Window->setEnabled(true);
    ui->actionZoom_In_->setEnabled(true);
    ui->actionZoom_Out->setEnabled(true);

    ui->select_ref_point_radioButton->setEnabled(true);
    ui->length_checkBox->setEnabled(true);
    ui->angle_checkBox->setEnabled(true);
    ui->refpoint_lineEdit->setEnabled(true);
} // open image

void MainWindow::on_actionSave_triggered()
{
    if(!check_imageOpened()){
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
    if(scene == NULL){
        errorMsg();
        return;
    }

    ui->graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->setAlignment(Qt::AlignCenter);
} // fit to window

void MainWindow::on_actionZoom_In__triggered()
{
    if (scene == NULL){
        errorMsg();
        return;
    } // error

    ui->graphicsView->scale(scaleFactor, scaleFactor);
} // zoom in

void MainWindow::on_actionZoom_Out_triggered()
{
    if (scene == NULL){
        errorMsg();
        return;
    } // error

    ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
} // zoom out

void MainWindow::on_actionView_Documentation_triggered()
{
    helpWin->show();
}

/**********************************************************************************/
/*********************** Main User Interface Functionalities **********************/
/**********************************************************************************/
void MainWindow::close_opencv_window(string window_name)
{
    while(true){
        char c = waitKey(33);
        if( c == 27 ){
            destroyWindow(window_name);
        }
    }//while not esc
}


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
    else if(ui->imageMode_comboBox->currentText() == "Edge") {
        src = imread(imagePath.toStdString());
        dst = edgeWin->setEdge(src, t);
        updateView(dst);
    }

}

void MainWindow::on_displayOrigImage_pushButton_clicked()
{
    if(!check_imageOpened()){
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
    if(!check_imageOpened()){
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
    else if(ui->imageMode_comboBox->currentText() == "Edge") {
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
    if(!check_imageOpened()){
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
        if (t > 0) {
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

// void MainWindow::on_edgeButton_clicked()
// {
//     if(!check_imageOpened()){
//         errorMsg();
//         return;
//     } // error

//     // get image to be displayed in edge mode
//     int index = ui->imageFiles_listWidget->currentRow();
//     imagePath = imagePaths.at(index);
//     src = imread(imagePath.toStdString());
//     cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));

//     //show edge window
//     edgeWin->setImageView(src_resize);
//     edgeWin->show();
// }

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
    if(check_imageOpened()){

        int index = ui->imageFiles_listWidget->currentRow();
        imagePath = imagePaths.at(index);

        if (mouseEnabled) {
            QPoint  local_pt = ui->graphicsView->mapFromGlobal(event->globalPos());
            QPointF img_coord_pt = ui->graphicsView->mapToScene(local_pt);

            qreal x_coord = img_coord_pt.x();
            qreal y_coord = img_coord_pt.y();
            // qDebug() << x_coord << y_coord;
            bloodVesselObject->saveTipPoint(imagePath.toStdString(), x_coord, y_coord);

            // adjusted based on reference point
            qreal selected_x = (qreal)(img_coord_pt.x() - src.cols/2)/(qreal)(src.cols/2);
            qreal selected_y = (qreal)(src.rows/2 - img_coord_pt.y())/(qreal)(src.rows/2);
            qreal x2_x1 = selected_x - ref_point.x();
            qreal y2_y1 = selected_y - ref_point.y();

            qreal length = 0, angle = 0;
            if(lengthEnabled)
                 length = sqrt(x2_x1 * x2_x1 + y2_y1 * y2_y1);

            if(angleEnabled) {
                //atan2 returns positive angle for positive Y position, and vice versa
                angle = atan2(y2_y1, x2_x1) * 180 / PI;
                if(angle < 0) //recalculate to get a positive angle value
                    angle = 360 + angle;
            }

            // each (x, y) point is displayed in their appropriate text edits
            QString x = QString::number((double) x2_x1, 'g', 3);
            QString y = QString::number((double) y2_y1, 'g', 3);
            QString l = QString::number((double)length, 'g', 3); //length
            QString a = QString::number((double)angle, 'g', 3); //angle
            QString e = " "; //empty

            //disregard clicks that were pressed outside of the image
            if (selected_x >= -1 && selected_x <= 1 && selected_y >= -1 && selected_y <= 1) {
                ui->actionUndo_Manual_Detect->setEnabled(true);
                ui->tipsXcoord_textEdit->append(x);
                ui->tipsYcoord_textEdit->append(y);
                if(lengthEnabled){
                    ui->length_textEdit->append(l);
                }
                else{
                    ui->length_textEdit->append(e);
                }

                if(angleEnabled) {
                    ui->angle_textEdit->append(a);
                }
                else {
                    ui->angle_textEdit->append(e);
                }
            }
            // display the tips in real time
            dst = bloodVesselObject->identifyTip(src, (float) x_coord, (float) y_coord);
            updateView(dst);
        }
        else if(refPointEnabled && !mouseEnabled){

                QPoint  local_pt = ui->graphicsView->mapFromGlobal(event->globalPos());
                QPointF img_coord_pt = ui->graphicsView->mapToScene(local_pt);
//                qDebug() << "local_pt = " << local_pt << ", img_coord_pt = " << img_coord_pt << endl;

                // adjusted based on reference point
                qreal adjusted_x = (qreal)(img_coord_pt.x() - src.cols/2)/(qreal)(src.cols/2);
                qreal adjusted_y = (qreal)(src.rows/2 - img_coord_pt.y())/(qreal)(src.rows/2);
//                qDebug() << "adjusted_x = " << adjusted_x << ", adjusted_y = " << adjusted_y << endl;
//                qDebug() << "the center? " << src.cols/2 << ", " << src.rows/2 << endl;

                // each (x, y) point is displayed in their appropriate text edits
                // (0, 0) is at the center of the image
                QString x = QString::number((double) adjusted_x, 'g', 3);
                QString y = QString::number((double) adjusted_y, 'g', 3);
//                qDebug() << "x = " << x << ", y = " << y << endl;
                QString ref = "Keep (" + x + ", " + y + ") as reference point?";

                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Reference Point", ref,
                                              QMessageBox::Yes|QMessageBox::No);
                if(reply == QMessageBox::Yes){
                    ref_point.setX(adjusted_x);
                    ref_point.setY(adjusted_y);
                    QString rx = QString::number((double)ref_point.x(), 'g', 3);
                    QString ry = QString::number((double)ref_point.y(), 'g', 3);

                    QString r = rx + ", " + ry;
                    ui->refpoint_lineEdit->setText(r);
                    refPointEnabled = false;
                    selected_ref = true;
                    if(revert){
                        mouseEnabled = true;
                        revert = false;
                    }
//                    qDebug() << "ref_point " << ref_point << endl;
                }
        }

    }

}

void MainWindow::on_bloodVesselsTips_radioButton_toggled(bool checked)
{
    if(!check_imageOpened()){
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
}

void MainWindow::on_displayTips_pushButton_clicked()
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if (!bloodVesselObject->isEmpty()) {
        ui->actionUndo_Manual_Detect->setEnabled(true);
        src = imread(imagePath.toStdString());
        dst = bloodVesselObject->displayTips(src, imagePath.toStdString());
        updateView(dst);
    }

}


void MainWindow::on_clearTips_pushButton_clicked()
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);

    if (!bloodVesselObject->isEmpty()) {
        src = imread(imagePath.toStdString());
    }
}

/**********************************************************************************/
/************************ Main Application Functionalities ************************/
/**********************************************************************************/

void MainWindow::on_tipDetect_pushButton_clicked()
{
    if(!check_imageOpened()){
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

} // tip detection

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
                if(lengthEnabled)
                   stream << ",Length";
                if(angleEnabled)
                   stream << ",Angle";
                stream << endl;

                QVector<QVector2D> pts = tips_map[temp]; // coordinates associated with image
                for (int i = 0; i < pts.size(); i++) {
                    QVector2D pt = pts.at(i);

                    //in automatic detection, below two values will already be in range [-1,1]
                    x2_x1 = pt.x() - ref_point.x();
                    y2_y1 = pt.y() - ref_point.y();

                    //if manual selection is true, must convert tip points into range [-1,1]
                    if(manualSelected == true) {
                        qreal selected_x = (qreal)(pt.x() - src.cols/2)/(qreal)(src.cols/2);
                        qreal selected_y = (qreal)(src.rows/2 - pt.y())/(qreal)(src.rows/2);
                        x2_x1 = selected_x - ref_point.x();
                        y2_y1 = selected_y - ref_point.y();
                    }

                    stream << x2_x1 << "," << y2_y1; // write all X, Y coordinates to file

                    qreal length = 0.0, angle = 0.0;
                    if(lengthEnabled) {
                        length = sqrt(x2_x1 * x2_x1 + y2_y1 * y2_y1);
                        stream << "," << length; //append length vaue to current row
                    }
                    if(angleEnabled) {
                        angle = atan2(y2_y1, x2_x1) * 180 / PI;

                        if(angle < 0)
                            angle = 360 + angle;

                        stream << "," << angle; //append angle value to current row
                    }

                    stream << endl;
                }
            }
        } //if file.open()
        file.close();
     }

    manualSelected = false;
} // write tips to file

void MainWindow::on_exportManual_pushButton_clicked() {
    manualSelected = true;
    unordered_map<string, QVector<QVector2D> > tips_map;
    bloodVesselObject->getManuallySelectedTips(tips_map);
    writeTipsToFile(tips_map);
}

void MainWindow::on_branchGraph_clicked()
{
    if(!check_imageOpened()){
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
} // branch graph

/**********************************************************************************/
/****************************** Slideshow Functions *******************************/
/**********************************************************************************/

void MainWindow::on_animate_pushButton_clicked()
{

    if(!check_imageOpened()){
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
    if(!check_imageOpened()){
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


//void MainWindow::on_tester_pushButton_clicked()
//{
//    if(!check_imageOpened()){
//        errorMsg();
//        return;
//    } // error

//    unordered_map<string, QVector<QVector2D> > test_map;

//    int index = ui->imageFiles_listWidget->currentRow(); // index of the selected image
//    imagePath = imagePaths.at(index); // absolute path of the selected image
//    string imName = imagePath.toStdString();
//    src = imread(imName);
//    cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));

//    edgeWin->detectTips(src_resize, test_map, imName, 135);
//    //writeTipsToFile(test_map);
//}

void MainWindow::on_select_ref_point_radioButton_clicked()
{

    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    refPointEnabled = true;
    if (mouseEnabled) {
        mouseEnabled = false;
        revert = true;
    }

}//select reference point

void MainWindow::on_tip_checkBox_clicked(bool checked)
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    if (checked)
        tipsEnabled = true;
    else
        tipsEnabled = false;
} //include tips

void MainWindow::on_length_checkBox_clicked(bool checked)
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error
//    lengthEnabled = true;
    if(checked){
        lengthEnabled = true;
    }
    else{
        lengthEnabled = false;
    }
}//include length

void MainWindow::on_angle_checkBox_clicked(bool checked)
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    if(checked){
        angleEnabled = true;
    }
    else{
        angleEnabled = false;
    }

}

void MainWindow::on_closeImage_toolButton_clicked()
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    int index = 0;
    if (imagePaths.size() > 1) {
        index = ui->imageFiles_listWidget->currentRow();

        if (index == 0) {
            imagePath = imagePaths.at(index + 1);
        }
        else {
            imagePath = imagePaths.at(index - 1);
        }

        src = imread(imagePath.toStdString());
        updateView(src);

    }
    else {
        Mat dummyImg = src;
        dummyImg.setTo(Scalar(255, 255, 255));
        updateView(dummyImg);
    }

    if (imageListPtr > 0) {
        imageListPtr--;
        src_images.remove(index);
        imagePaths.removeAt(index);
        tips_map.erase(imagePath.toStdString());
        thresholds.erase(imagePath.toStdString());
        ui->imageFiles_listWidget->takeItem(index);
    }

} // close current image on display

void MainWindow::on_actionUndo_Manual_Detect_triggered()
{
    // if there is an image
    if (imageListPtr > 0) {
        int index = ui->imageFiles_listWidget->currentRow();
        imagePath = imagePaths.at(index);
        string imp = imagePath.toStdString();
        src = imread(imp);

        if (!bloodVesselObject->thisImageTipsIsEmpty(imp)) {
            bloodVesselObject->deleteTipPoint(imp);

            QStringList xbox_lines = (ui->tipsXcoord_textEdit->toPlainText()).split("\n");
            xbox_lines.removeLast();
            ui->tipsXcoord_textEdit->setPlainText(xbox_lines.join("\n"));

            QStringList ybox_lines = (ui->tipsYcoord_textEdit->toPlainText()).split("\n");
            ybox_lines.removeLast();
            ui->tipsYcoord_textEdit->setPlainText(ybox_lines.join("\n"));

            if (bloodVesselObject->thisImageTipsIsEmpty(imp)) {
                ui->actionUndo_Manual_Detect->setDisabled(true);
            }
            dst = bloodVesselObject->displayTips(src, imagePath.toStdString());
            updateView(dst);
        }
    }
}



