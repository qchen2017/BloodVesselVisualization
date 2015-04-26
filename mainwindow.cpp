#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QVector2D>
#include <QVector>

using namespace cv;
using namespace std;

<<<<<<< HEAD
//global variables for slide show
int trackbarNum, timeDelay, slideNum;
bool slidePause;
Mat imageRead;
QStringList pathlists;
QString pathname;
char keyPressed;
=======
>>>>>>> origin/master

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ssWin = new Slideshow(this);
    edgeWin = new Edge(this); // separate UI for Edge mode
    helpWin = new QWidget; // help window
    helpWin->sizeHint();
    helpWin->setWindowTitle("Help");
    ui->setupUi(this); // main window UI
    ui->threshold_lineEdit->setText("0"); // initialize line edit for the threshold value

    // create pointers for image and blood vessel objects
    imagePtr = new Image();
    bloodVesselObject = new BloodVessels(this); // also has a separate UI

    // initialize variables for threshold and zoom functions
    scaleFactor = 1.15;

    // use for manual detection of blood vessel tips
     mouseEnabled = false;

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

    static QLabel helpInfo;
    helpInfo.setText("Design Document: https://docs.google.com/a/ucdavis.edu/document/d/1MzFV0zI-LZV6j7tqBh1H0ese6fr5gMz8w4HmYBFGPOk/\n\n"
                     "Testing Document: https://docs.google.com/a/ucdavis.edu/document/d/1hkfqfILpR68mZEYMrsZXgx0fQEmJTdX65G3fDLlw8MY/");
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
    QMessageBox::about(this, tr("Error !"),
                       tr("No loaded image.\n"
                          "Please open an image first.\n"));

} // error message for no image

bool MainWindow::check_imageOpened()
{
    if(imagePath == NULL)
        return false;
    return true;

} // check if an image has been loaded

void MainWindow::on_actionOpen_triggered()
{
    // accepts png and jpg image types
    imagePaths = QFileDialog::getOpenFileNames(
                            this, tr("Select one or more files to open"), "",
                            tr("Images (*.png *.jpeg *.jpg)"));

    // checks if image is valid
    if (imagePaths.isEmpty()) {
        errorMsg();
        return;
    }

    // stores all the absolute paths of all the images that are being loaded
    for (int i = 0; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        if (imagePath == NULL) { // alert user if there's an error with one or more of the images
            QMessageBox::about(this, tr("Error !"),
                               tr("Error loading file."));
            imagePaths.removeAt(i);
            i--;
        }
        else { // insert image into image files list
            ui->imageFiles_listWidget->insertItem(i, imagePath);
            Mat img = imread(imagePath.toStdString());
            if (i == 0) {
                src = img; // set current src image
            }
            src_images.push_back(img);
            thresholds[imagePath.toStdString()] = 0;
        }
    }

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
    ui->menuView->setEnabled(true);
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

void MainWindow::on_imageFiles_listWidget_itemClicked(QListWidgetItem *item)
{
    item->setSelected(true); // the selected image from the list
    int index = ui->imageFiles_listWidget->currentRow(); // index of the selected image
    imagePath = imagePaths.at(index); // absolute path of the selected image
    src = imread(imagePath.toStdString());
    int t = thresholds[imagePath.toStdString()];
    ui->threshold_horizontalSlider->setValue(t);

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
        Mat contourOut = imagePtr->setImageView(src, value, "contour");
        updateView(contourOut);
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
}

void MainWindow::on_edgeButton_clicked()
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    // get image to be displayed in edge mode
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    src = imread(imagePath.toStdString());
    cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));

    //show edge window
    edgeWin->setImageView(src_resize);
    edgeWin->show();
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
            qreal adjusted_x = (qreal)(img_coord_pt.x() - src.cols/2)/(qreal)(src.cols);
            qreal adjusted_y = (qreal)(src.rows/2 - img_coord_pt.y())/(qreal)(src.rows);

            // each (x, y) point is displayed in their appropriate text edits
            // (0, 0) is at the center of the image
            QString x = QString::number((double) adjusted_x, 'g', 3);
            QString y = QString::number((double) adjusted_y, 'g', 3);
            ui->tipsXcoord_textEdit->append(x);
            ui->tipsYcoord_textEdit->append(y);

            // display the tips in real time
            dst = bloodVesselObject->identifyTip(src, (float) x_coord, (float) y_coord);
            updateView(dst);
        }
    }

}

void MainWindow::on_bloodVesselsTips_radioButton_toggled(bool checked)
{
    int index = ui->imageFiles_listWidget->currentRow();
    imagePath = imagePaths.at(index);
    int t = thresholds[imagePath.toStdString()];

    if (checked) {
        src = imread(imagePath.toStdString());
        cv::threshold(src, dst, t, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C);
        mouseEnabled = true;
    }
    else {
        mouseEnabled = false;
        return;
    }
}

//void MainWindow::on_displayBloodVesselTips_radioButton_toggled(bool checked)
//{
//    int index = ui->imageFiles_listWidget->currentRow();
//    imagePath = imagePaths.at(index);

//    if (checked && !bloodVesselObject->isEmpty()) {
//        src = imread(imagePath.toStdString());
//        dst = bloodVesselObject->displayTips(src);
//        updateView(dst);
//    }
//}

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
    else {
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
                QVector<QVector2D> pts = tips_map[temp]; // coordinates associated with image
                for (int i = 0; i < pts.size(); i++) {
                    QVector2D pt = pts.at(i);
                    stream << pt.x() << "," << pt.y() << endl; // write all coordinates to file
                }
            }
        }
        file.close();
     }

} // write tips to file

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

   ssWin->setImageList(imagePaths);
   ssWin->show();
}

void MainWindow::promptForTipsAnimation(unordered_map<string, QVector<QVector2D> > &tips_map_temp)
{
    // prompt user to manually select threshold value for each image or let program do it
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Threshold Value", "Would you like to use the current threshold value for all of the images?",
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) { // currently set threshold value for each image
        QMessageBox::information(this, tr("Visualization of Directional Blood Vessels"),
                tr("The application will use the currently set threshold value for each image. "
                   "If you would like to set different threshold values, click 'Cancel' on the next window to go back.") );

        findAllTips(false, tips_map_temp);
    }
    else {
        findAllTips(true, tips_map_temp);
    }
}

void MainWindow::findAllTips(bool threshold_default, unordered_map<string, QVector<QVector2D> > &tips_map_temp)
{
    for (int i = 0; i < imagePaths.size(); i++) {
        imagePath = imagePaths.at(i);
        src = imread(imagePath.toStdString());
        cv::resize(src, src_resize, cv::Size2i(src.cols/3, src.rows/3));
        string imName = imagePath.toStdString();
        if (!threshold_default) {
            edgeWin->detectTips(src_resize, tips_map_temp, imName, thresholds[imName]);
        }
        else {
            edgeWin->detectTips(src_resize, tips_map_temp, imagePath.toStdString(), 135);
        }
        edgeWin->convertToPixelCoords(src_resize, tips_map_temp);

    }
}


void MainWindow::on_tipsAnimation_pushButton_clicked()
{
    if(!check_imageOpened()){
        errorMsg();
        return;
    } // error

    Mat tipsMovie = src;
    unordered_map<string, QVector<QVector2D> > tips_map_temp;

    // both are unchecked
    if (!ui->automated_checkBox->isChecked() && !ui->manual_checkBox->isChecked()) {
        // alert user to make a selection
        QMessageBox::about(this, tr("Error !"), tr("Please make a selection on which tips to animate."));
    }

    // only automated is checked
    else if (ui->automated_checkBox->isChecked() && !ui->manual_checkBox->isChecked())  {

        // RUN THE PROCESS OF AUTOMATICALLY DETECTING TIPS
        promptForTipsAnimation(tips_map_temp);

        bloodVesselObject->tipsAnimation(tipsMovie, tips_map_temp);
        bloodVesselObject->show();
    }
    // only manual is checked
    else if (ui->manual_checkBox->isChecked() && !ui->automated_checkBox->isChecked()) {
        if (!bloodVesselObject->isEmpty()) {
            bloodVesselObject->getManuallySelectedTips(tips_map_temp);
            bloodVesselObject->tipsAnimation(tipsMovie, tips_map_temp);
            bloodVesselObject->show();
        }
        else {
            QMessageBox::information(
                this,
                tr("Visualization of Directional Blood Vessels"),
                tr("No points have been selected. Nothing to show here.") );
        }
    }
    // both are checked
    else {

        // put all automated tips to tips_map_temp
        promptForTipsAnimation(tips_map_temp);

        // now add the manually selected ones, if any
        unordered_map<string, QVector<QVector2D> > tips_map_temp2;
        if (!bloodVesselObject->isEmpty()) {
            bloodVesselObject->getManuallySelectedTips(tips_map_temp2);
            for(auto it = tips_map_temp2.begin(); it != tips_map_temp2.end(); ++it) {
                string imname = it->first; // image path name
                QVector<QVector2D> temp_pts = tips_map_temp[imname];
                QVector<QVector2D> temp_pts2 = tips_map_temp2[imname];
                for (int i = 0; i < temp_pts2.size(); i++) {
                    temp_pts.push_back(temp_pts2.at(i));
                }
                tips_map_temp[imname] = temp_pts;
            }
        }

    }
} // tips animation
