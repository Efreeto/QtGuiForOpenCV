#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnPauseResume_clicked();

private:
    Ui::MainWindow *ui;

    // OpenCV class
    cv::VideoCapture capture;
    cv::Mat original;
    cv::Mat processed;
    // OpenCV variables
    std::vector<cv::Vec3f> vecCircles;
    std::vector<cv::Vec3f>::iterator itrCircles;
    double param1,param2;
    int minRadius,maxRadius;

    // QImage class
    QImage qimgOriginal;
    QImage qimgProcessed;

    // QTimer class - to give time to respond to UI
    QTimer *tmrTimer;

public slots:
    void processFrameAndUpdateGUI();
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // MAINWINDOW_H
