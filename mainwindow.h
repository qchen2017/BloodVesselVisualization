#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QListWidgetItem>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "image.h"
#include "bloodvessels.h"
#include "edge.h"

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

//public slots:
//    void mousePressEvent(QMouseEvent *event);

private slots:
    void errorMsg();

    /* Menu Bar Functions */
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionFit_to_Window_triggered();
    void on_actionActual_Size_triggered();
    void on_actionZoom_In__triggered();
    void on_actionZoom_Out_triggered();

    /* Main User Interface Functionalities */
    void on_imageFiles_listWidget_itemClicked(QListWidgetItem *item);
    void on_displayOrigImage_pushButton_clicked();
    void on_threshold_horizontalSlider_valueChanged(int value);
    void on_imageMode_comboBox_activated(const QString &arg1);
    void on_edgeButton_clicked();
//    void on_bloodVesselsTips_radioButton_toggled(bool checked);
//    void on_displayBloodVesselTips_radioButton_toggled(bool checked);

    /* Main App Function */
    void on_tipDetect_pushButton_clicked();
    void on_branchGraph_clicked();
    void on_animate_pushButton_clicked();

private:

    void writeTipsToFile(unordered_map<string, QVector<QVector2D> > tips_map);

    Ui::MainWindow *ui;

    QStringList imagePaths;
    QString imagePath;
    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    Image *imagePtr;
    BloodVessels *bloodVesselObject;
    Edge *edgeWin;

    Mat src, src_resize, dst;
    QVector<Mat> src_images;

    int threshold_val;
    float scaleFactor;

    bool mouseEnabled;


};

#endif // MAINWINDOW_H
