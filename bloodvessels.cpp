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

void BloodVessels::tipsAnimation(unordered_map<string, QVector<QVector2D> > tips_map)
{

    Mat img;
    QVector2D pt;
    int x, y;

    for(auto it = tips_map.begin(); it != tips_map.end(); ++it) {
        string temp = it->first; // image path name
        QVector<QVector2D> pts = tips_map[temp]; // coordinates associated with image
        img = imread(temp);
        img.setTo(cv::Scalar(0, 0, 0)); // fill image with black
        for (int i = 0; i < pts.size(); i++) {
           pt = pts.at(i);
           x = pt.x();
           y = pt.y();
           Point dot = Point(x, y);
           circle(img, dot, 10.0, Scalar(0, 0, 255), -1, 8);
        }
        cv::resize(img, img, cv::Size2i(img.cols/3, img.rows/3));
        tips_images.push_back(img);
    }
}

QVector<Mat> BloodVessels::getTipsImages()
{
    return tips_images;
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

Mat BloodVessels::identifyTip(Mat src, float x, float y, bool mouseEnabled, bool refPointEnabled)
{
    Point dot = Point(x, y);
    int thickness = -1;
    int lineType = 8;

    if(mouseEnabled)
        circle(src, dot, 20.0, Scalar(0, 0, 255), thickness, lineType);
    if(refPointEnabled)
        rectangle(src, Point(x-20, y-20), Point(x+20, y+20), Scalar(0, 0, 255), thickness, lineType);

    return src;
}

Mat BloodVessels::displayTips(Mat src, string imName, bool tipsEnabled, bool lengthEnabled, bool angleEnabled)
{
    Mat imageOut = src;

    QVector<QVector2D> bloodVesselsTips;
    if (bv_tips_map.find(imName) != bv_tips_map.end()) {
        bloodVesselsTips = bv_tips_map[imName];

        for (int i = 0; i < bloodVesselsTips.size(); i++) {
            float x = (float) bloodVesselsTips[i].x();
            float y = (float) bloodVesselsTips[i].y();
            Point dot = Point(x, y);
            if(tipsEnabled)
                circle(imageOut, dot, 20.0, Scalar(0, 0, 255), -1, 8);
            if(lengthEnabled)
                line(imageOut, Point(0,0), dot, Scalar(0, 0, 255), 1, 8);
        }
    }

    return imageOut;
}

void BloodVessels::updateView(Mat imageOut)
{
    // convert Mat to QImage display on graphicsView
    cv::cvtColor(imageOut, imageOut, cv::COLOR_BGR2RGB);
    QImage img((uchar*)imageOut.data, imageOut.cols, imageOut.rows, QImage::Format_RGB888);
    image = QPixmap::fromImage(img);
    scene = new QGraphicsScene(this);
    scene->addPixmap(image);
    scene->setSceneRect(0, 0, image.width(), image.height());
    ui->graphicsView->setScene(scene);
} // update graphic view
