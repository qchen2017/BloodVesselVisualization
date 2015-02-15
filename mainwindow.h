#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "imagemodes.h"

using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updateView(Mat imageOut);

    bool check_imageOpened();

    Mat setContours();
private slots:
    void errorMsg();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionFit_to_Window_triggered();

    void on_actionActual_Size_triggered();

    void on_actionZoom_In__triggered();

    void on_actionZoom_Out_triggered();



    void on_threshold_horizontalSlider_valueChanged(int value);

    void on_imageMode_comboBox_activated(const QString &arg1);

private:
    Ui::MainWindow *ui;

    QString imagePath;
    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    Contour *contour;
    Edge *edge;

    Mat src, dst;
    int threshold_val;
    float scaleFactor;
};

#endif // MAINWINDOW_H
