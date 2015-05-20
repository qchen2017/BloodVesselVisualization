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

    Mat img, img_orig_bg;
    QVector2D pt;
    int x, y;

    for(auto it = tips_map.begin(); it != tips_map.end(); ++it) {
        string temp = it->first; // image path name
        QVector<QVector2D> pts = tips_map[temp]; // coordinates associated with image
        img = imread(temp);
        img_orig_bg = imread(temp); // orig image as background
        img.setTo(cv::Scalar(0, 0, 0)); // fill image with black
        for (int i = 0; i < pts.size(); i++) {
           pt = pts.at(i);
           x = pt.x();
           y = pt.y();
           Point dot = Point(x, y);
           circle(img, dot, 20.0, Scalar(105, 105, 105), -1, 8);
           circle(img_orig_bg, dot, 20.0, Scalar(105, 105, 105), -1, 8);

        }
        cv::resize(img, img, cv::Size2i(img.cols/3, img.rows/3));
        tips_images.push_back(img);
        tips_images_orig_bg.push_back(img_orig_bg);
    }
}

QVector<Mat> BloodVessels::getTipsImages()
{
    return tips_images;
}

QVector<Mat> BloodVessels::getTipsImagesWithOrigBG()
{
    return tips_images_orig_bg;
}

void BloodVessels::clearImageVectors()
{
    tips_images.clear();
    tips_images_orig_bg.clear();
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

bool BloodVessels::thisImageTipsIsEmpty(string imgpath)
{
    if (bv_tips_map.find(imgpath) != bv_tips_map.end()) {
        QVector<QVector2D> temp = bv_tips_map[imgpath];
        if (temp.empty()) {
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

void BloodVessels::saveTipPoint(string imgpath, qreal x, qreal y)
{
    // update map
    QVector<QVector2D> temp;
    if (bv_tips_map.find(imgpath) != bv_tips_map.end()) {
        temp = bv_tips_map[imgpath];
    }
    temp.push_back(QVector2D(x, y));
    bv_tips_map[imgpath] = temp;
}

void BloodVessels::deleteTipPoint(string imgpath)
{
    // update map
    QVector<QVector2D> temp = bv_tips_map[imgpath];
    temp.remove(temp.size() - 1); // remove last element

    if (temp.empty()) {
        bv_tips_map.erase(imgpath);
    }
    else {
        bv_tips_map[imgpath] = temp;
    }
}

void BloodVessels::deleteAllTipPoints(string imgpath)
{
    if (!imgpath.empty()) {
        bv_tips_map.erase(imgpath);
    }
    else {
        unordered_map<string, QVector<QVector2D> > bv_tips_map_cleared;
        bv_tips_map = bv_tips_map_cleared;
    }
}

Mat BloodVessels::identifyTip(Mat src, float x, float y, int tips_size, QColor tips_color, QPointF refPtPixel)
{
    Point dot = Point(x, y);
    int thickness = -1;
    int lineType = 8;
    int red = tips_color.red();
    int green = tips_color.green();
    int blue = tips_color.blue();

    circle(src, dot, (float)tips_size, Scalar(blue, green, red), thickness, lineType);
    int ref_x = refPtPixel.x();
    int ref_y = refPtPixel.y();
    rectangle(src, Point(ref_x-tips_size, ref_y-tips_size), Point(ref_x+tips_size, ref_y+tips_size), Scalar(red, green, blue), thickness, lineType);

    return src;
}

Mat BloodVessels::displayTips(Mat src, string imName, int tips_size, QColor tips_color, QPointF refPtPixel, bool lengthEnabled, bool tipsEnabled)
{
    Mat imageOut = src;
    int red = tips_color.red();
    int green = tips_color.green();
    int blue = tips_color.blue();

    int ref_x = refPtPixel.x();
    int ref_y = refPtPixel.y();

    QVector<QVector2D> bloodVesselsTips;
    if (bv_tips_map.find(imName) != bv_tips_map.end()) {
        bloodVesselsTips = bv_tips_map[imName];

        for (int i = 0; i < bloodVesselsTips.size(); i++) {
            float x = (float) bloodVesselsTips[i].x();
            float y = (float) bloodVesselsTips[i].y();
            Point dot = Point(x, y);
            if (tipsEnabled) {
                circle(imageOut, dot, (float)tips_size, Scalar(blue, green, red), -1, 8);
            }
            if (lengthEnabled) {
                line(imageOut, Point(ref_x, ref_y), dot, Scalar(blue, green, red), 5, 8);
            }
        }
    }

    rectangle(imageOut, Point(ref_x-tips_size, ref_y-tips_size), Point(ref_x+tips_size, ref_y+tips_size), Scalar(red, green, blue), -1, 8);

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
