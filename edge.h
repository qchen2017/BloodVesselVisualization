#ifndef EDGE_H
#define EDGE_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <unordered_map>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace std;

namespace Ui {
class Edge;
}

class Edge : public QMainWindow
{
    Q_OBJECT

public:
    Edge(QWidget *parent = 0);
    ~Edge();

    void setImageView(cv::Mat imageOut); // set up image for edge mode
    cv::Mat setEdge(cv::Mat imageIn, int thresh_val);
    cv::Mat detectTips(cv::Mat imageIn, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName, int thresh_val); // set up image for tips detection
    void branchGraph(cv::Mat imageIn, cv::Mat &imageOut);
    void convertToPixelCoords(cv::Mat imageIn, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName);
    void getAutoTipsImages(QVector<cv::Mat> &auto_tips_images);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_varySlider_valueChanged(int value);

private:
    Ui::Edge *ui; // currently not used

    // functions for edge mode
    void updateView(cv::Mat imageOut);

    // functions for tips detection
    void GetLutSkel(cv::Mat &Lut);
    void applylut_1(cv::Mat &imageIn, cv::Mat &imageOut);
    void applylut_8(cv::Mat &src, cv::Mat &dst, cv::Mat &lut);
    void skel(cv::Mat &src, cv::Mat &dst);
    void endp(cv::Mat &imageIn, cv::Mat &imageOut, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName);

    // function for getting tips coordinate
    void getTipsCoords(cv::Mat imageIn, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName);

    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    cv::Mat src, src_gray, dst;
    int thresh = 0;
    QVector<cv::Mat> automated_tips_images;
};

#endif // EDGE_H
