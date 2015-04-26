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

using namespace cv;
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

    void setImageView(Mat imageOut); // set up image for edge mode
    Mat detectTips(Mat imageIn, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName, int thresh_val); // set up image for tips detection
    void branchGraph(Mat imageIn, Mat &imageOut);

    void convertToPixelCoords(Mat imageIn, unordered_map<string, QVector<QVector2D> > &tips_map);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_varySlider_valueChanged(int value);

private:
    Ui::Edge *ui;

    // functions for edge mode
    Mat setEdge();
    void updateView(Mat imageOut);

    // functions for tips detection
    void GetLutSkel(Mat &Lut);
    void applylut_1(Mat &imageIn, Mat &imageOut);
    void applylut_8(Mat &src, Mat &dst, Mat &lut);
    void skel(Mat &src, Mat &dst);
    void endp(Mat &imageIn, Mat &imageOut, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName);

    // functions for getting tips coordinate
    void getTipsCoords(Mat imageIn, unordered_map<string, QVector<QVector2D> > &tips_map, string imgName);

    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    //variables
    Mat src, src_gray, dst;
    int thresh = 0;
};

#endif // EDGE_H
