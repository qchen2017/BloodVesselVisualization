#ifndef EDGE_H
#define EDGE_H

#include <QWidget>
#include <QMainWindow>
#include <QGraphicsScene>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;

namespace Ui {
class Edge;
}

class Edge : public QMainWindow
{
    Q_OBJECT

public:
    Edge(QWidget *parent = 0);
    ~Edge();

    void setImageView(Mat imageOut);
    void updateThreshold();
    Mat detectTips(Mat imageIn);

    void GetLutSkel(Mat &Lut);
    void applylut_1(Mat &src, Mat &dst);
    void applylut_8(Mat &src, Mat &dst, Mat &lut);
    void skel(Mat &src, Mat &dst);
    void endp(Mat &src, Mat &dst);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_varySlider_valueChanged(int value);

private:
    Ui::Edge *ui;

    void updateView(Mat imageOut);
    Mat setEdge();

    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    //variables
    Mat src, src_gray, dst;
    int thresh = 0;
};

#endif // EDGE_H
