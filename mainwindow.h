#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QListWidgetItem>
#include <unordered_map>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "image.h"
#include "bloodvessels.h"
#include "edge.h"
#include "slideshow.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updateView(cv::Mat imageOut);
    bool check_imageOpened();


public slots:
    void mousePressEvent(QMouseEvent *event);

private slots:
    void errorMsg();

    /* Menu Bar Functions */
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionFit_to_Window_triggered();
    void on_actionZoom_In__triggered();
    void on_actionZoom_Out_triggered();
    void on_actionView_Documentation_triggered();

    /* Main User Interface Functionalities */
    void close_opencv_window(string window_name);
    void on_imageFiles_listWidget_itemClicked(QListWidgetItem *item);
    void on_displayOrigImage_pushButton_clicked();
    void on_threshold_horizontalSlider_valueChanged(int value);
    void on_imageMode_comboBox_activated(const QString &arg1);
    //void on_edgeButton_clicked();
    void on_bloodVesselsTips_radioButton_toggled(bool checked);
    void on_displayTips_pushButton_clicked();
    void on_clearTips_pushButton_clicked();

    /* Main App Functions */
    void on_tipDetect_pushButton_clicked();
    void on_branchGraph_clicked();
    void on_animate_pushButton_clicked();
    void on_tipsAnimation_pushButton_clicked();

//    void on_tester_pushButton_clicked();

    /* tips function */
    void on_select_ref_point_radioButton_clicked(bool checked);
    void on_length_checkBox_clicked(bool checked);
    void on_tip_checkBox_clicked(bool checked);
    void on_angle_checkBox_clicked(bool checked);    

private:
    void writeTipsToFile(unordered_map<string, QVector<QVector2D> > tips_map);

    Ui::MainWindow *ui;
    QWidget *helpWin;

    QStringList imagePaths;
    QString imagePath;
    QPixmap image;
    QImage *imageObject;
    QGraphicsScene *scene;

    Image *imagePtr;
    BloodVessels *bloodVesselObject;
    Edge *edgeWin;
    Slideshow *ssWin;

    cv::Mat src, src_resize, dst, contourOut, edgeOut;
    QVector<cv::Mat> src_images;

    unordered_map<string, int> thresholds;
    float scaleFactor;
    bool mouseEnabled;
    int imageListPtr;

    bool tipsEnabled;
    bool angleEnabled;
    bool lengthEnabled;
    bool refPointEnabled;
    bool selected_ref;
    QVector2D ref_point;

    // string holds the name of the image
    // vector of QVector2D holds the x and y coordinates of the tips in the image
    unordered_map<string, QVector<QVector2D> > tips_map;

    void findAllTips(bool threshold_default, unordered_map<string, QVector<QVector2D> > &tips_map_temp);
    void promptForTipsAnimation(unordered_map<string, QVector<QVector2D> > &tips_map_temp);
    void automatedTipsAnimation(QVector<cv::Mat> &auto_tips_images, unordered_map<string, QVector<QVector2D> > &tips_map_temp);

    bool imageAlreadyLoaded(QString imp);
};

#endif // MAINWINDOW_H
