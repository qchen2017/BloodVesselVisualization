#include "bloodvessels.h"
#include "ui_bloodvessels.h"
#include <QVector3D>
#include <QDebug>

using namespace cv;
using namespace std;

BloodVessels::BloodVessels(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BloodVessels)
{
    ui->setupUi(this);
    setFixedSize(size().width(), size().height());

} // constructor

BloodVessels::~BloodVessels()
{
    delete ui;
}

void BloodVessels::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void BloodVessels::tipsAnimation(Mat imageIn, unordered_map<string, QVector<QVector2D> > tips_map)
{
    // img needs to be a movie
    // or need a vector of img to play

    Mat img;
    cv::resize(imageIn, img, cv::Size2i(imageIn.cols/3, imageIn.rows/3));
    // need a better way of filling image with black
    //img.setTo(cv::Scalar(0, 0, 0)); // fill image with black
    cvtColor(img, img, cv::COLOR_BGR2GRAY);


    QVector2D pt;
    int x, y;

    for(auto it = tips_map.begin(); it != tips_map.end(); ++it) {
        string temp = it->first; // image path name
        QVector<QVector2D> pts = tips_map[temp]; // coordinates associated with image
        for (int i = 0; i < pts.size(); i++) {
           pt = pts.at(i);
           x = pt.x();
           y = pt.y();
           Point dot = Point(x, y);
           circle(img, dot, 5.0, Scalar(255, 0, 0), -1, 8);
        }
    }

    updateView(img);


}

void BloodVessels::getManuallySelectedTips(unordered_map<string, QVector<QVector2D> > &tips)
{
    tips = bv_tips_map;
}

bool BloodVessels::isEmpty()
{
    if (bv_tips_map.empty()) {
        return true;
    }
    return false;
}

void BloodVessels::saveTipPoint(string imgpath, qreal x, qreal y)
{
    // update map
    QVector<QVector2D> temp = bv_tips_map[imgpath];
    temp.push_back(QVector2D(x, y));
    bv_tips_map[imgpath] = temp;
}

Mat BloodVessels::identifyTip(Mat src, float x, float y)
{
    Point dot = Point(x, y);
    int thickness = -1;
    int lineType = 8;

     circle(src,
            dot,
            25.0,
            Scalar(0, 0, 255),
            thickness,
            lineType);

     return src;
}

Mat BloodVessels::displayTips(Mat src, string imName)
{
    Mat imageOut = src;
    int thickness = -1;
    int lineType = 8;

    QVector<QVector2D> bloodVesselsTips;
    bloodVesselsTips = bv_tips_map[imName];

    for (int i = 0; i < bloodVesselsTips.size(); i++) {
        float x = (float) bloodVesselsTips[i].x();
        float y = (float) bloodVesselsTips[i].y();
        Point dot = Point(x, y);
        circle(src,
               dot,
               20.0,
               Scalar(0, 0, 255),
               thickness,
               lineType);
    }

    return imageOut;
}

void BloodVessels::updateView(Mat imageOut)
{
    // convert Mat to QImage display on graphicsView
    QImage img((uchar*)imageOut.data, imageOut.cols, imageOut.rows, QImage::Format_Indexed8);
    image = QPixmap::fromImage(img);
    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());
    ui->graphicsView->setScene(scene);
} // update graphic view

void BloodVessels::on_start_pushButton_clicked()
{

}
