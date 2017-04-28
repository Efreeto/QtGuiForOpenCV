#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Maximize window on start
    this->setWindowState(Qt::WindowMaximized);

    // Open OpenCV capture
    capture.open(0);
    // Check if sucessful
    if(!capture.isOpened() == true){
        ui->txtXYRadius->appendPlainText("error: camera error!");
        return;
    }

    // Set initial time
    ui->txtLoopTime->appendPlainText(QString("20"));

    // Timer for UI responsivness
    tmrTimer = new QTimer(this);
    connect(tmrTimer,SIGNAL(timeout()),this,SLOT(processFrameAndUpdateGUI()));
    tmrTimer->start(ui->txtLoopTime->toPlainText().toInt()); //ui->txtLoopTime->toPlainText().toInt() msec

    // Set initial sliders postion (for tracking blue objects on white background, using BGR)
    ui->SldBlueLower->setSliderPosition(0);
    ui->SldBlueUpper->setSliderPosition(255);
    ui->SldGreenLower->setSliderPosition(0);
    ui->SldGreenUpper->setSliderPosition(255);
    ui->SldRedLower->setSliderPosition(0);
    ui->SldRedUpper->setSliderPosition(115);

    // Set initial values for Hough circle detection
    param1 = 70.0;
    param2 = 20.0;
    minRadius = 20;
    maxRadius = 200;
    // Update text
    ui->txtparam1->appendPlainText(QString::number(param1,'f',1));
    ui->txtparam2->appendPlainText(QString::number(param2,'f',1));
    ui->txtminRadius->appendPlainText(QString::number(minRadius,'d',0));
    ui->txtmaxRadius->appendPlainText(QString::number(maxRadius,'d',0));
    // Install event filter
    ui->txtparam1->installEventFilter(this);
    ui->txtparam2->installEventFilter(this);
    ui->txtminRadius->installEventFilter(this);
    ui->txtmaxRadius->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processFrameAndUpdateGUI(){
    capture.read(original);

    if(original.empty() == true)
        return;

    // Text box to see slider values (can be used as an event for efficiency)
    ui->SldBlueValue->clear();
    ui->SldBlueValue->appendPlainText(QString("(") +
                                      QString::number(ui->SldBlueLower->value()).rightJustified(3,' ') +
                                      QString(",") +
                                      QString::number(ui->SldBlueUpper->value()).rightJustified(3,' ') +
                                      QString(")"));
    ui->SldGreenValue->clear();
    ui->SldGreenValue->appendPlainText(QString("(") +
                                       QString::number(ui->SldGreenLower->value()).rightJustified(3,' ') +
                                       QString(",") +
                                       QString::number(ui->SldGreenUpper->value()).rightJustified(3,' ') +
                                       QString(")"));
    ui->SldRedValue->clear();
    ui->SldRedValue->appendPlainText(QString("(") +
                                     QString::number(ui->SldRedLower->value()).rightJustified(3,' ') +
                                     QString(",") +
                                     QString::number(ui->SldRedUpper->value()).rightJustified(3,' ') +
                                     QString(")"));

    // Check inRange based on the slider values
    cv::inRange(original,cv::Scalar(ui->SldBlueLower->value(),ui->SldGreenLower->value(),ui->SldRedLower->value()),
                         cv::Scalar(ui->SldBlueUpper->value(),ui->SldGreenUpper->value(),ui->SldRedUpper->value()),processed);
    // Blur image
    cv::GaussianBlur(processed,processed,cv::Size(9,9),1.5);
    // Detect Hough Circles
    cv::HoughCircles(processed,vecCircles,CV_HOUGH_GRADIENT,1,processed.rows / 8,param1,param2,minRadius,maxRadius);
    // Vector iterator - iterate through results, and overlay the circles and add text (center, radius)
    for(itrCircles = vecCircles.begin();itrCircles != vecCircles.end();itrCircles++){
        ui->txtXYRadius->appendPlainText(QString("position x=") + QString::number((*itrCircles)[0]).rightJustified(4,' ') +
                                         QString(", y=") + QString::number((*itrCircles)[1]).rightJustified(4,' ') +
                                         QString(", radius=") + QString::number((*itrCircles)[2],'f',3).rightJustified(7,' '));

        cv::circle(original,cv::Point((int)(*itrCircles)[0],(int)(*itrCircles)[1]),3,cv::Scalar(0,255,0),CV_FILLED);
        cv::circle(original,cv::Point((int)(*itrCircles)[0],(int)(*itrCircles)[1]),(int)(*itrCircles)[2],cv::Scalar(0,255,0),3);
    }

    // OpenCV to QImage datatype to display on labels
    cv::cvtColor(original,original,CV_BGR2RGB);
    QImage qimgOriginal((uchar*) original.data,original.cols,original.rows,original.step,QImage::Format_RGB888); // for color images
    QImage qimgProcessed((uchar*) processed.data,processed.cols,processed.rows,processed.step,QImage::Format_Indexed8); // for grayscale images

    // Update the labels on the form
    ui->lblOriginal->setPixmap(QPixmap::fromImage(qimgOriginal));
    ui->lblProcessed->setPixmap(QPixmap::fromImage(qimgProcessed));
}

// Button for pause stream
void MainWindow::on_btnPauseResume_clicked()
{
    if(tmrTimer->isActive() == true){ // timer is running
        tmrTimer->stop();
        ui->btnPauseResume->setText("resume stream");
    }
    else{
        tmrTimer->start(ui->txtLoopTime->toPlainText().toInt());
        ui->btnPauseResume->setText("pause stream");
    }
}

// Handle event filter (enter button press)
bool MainWindow::eventFilter(QObject *object, QEvent *event){
    if(((object == ui->txtparam1) ||
       (object == ui->txtparam2) ||
       (object == ui->txtminRadius) ||
       (object == ui->txtmaxRadius))&&
        event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Return  || keyEvent->key() == Qt::Key_Enter){
            // Read the values
            param1 = ui->txtparam1->toPlainText().toDouble();
            param2 = ui->txtparam2->toPlainText().toDouble();
            minRadius = ui->txtminRadius->toPlainText().toInt();
            maxRadius = ui->txtmaxRadius->toPlainText().toInt();
        }
    }
    return QMainWindow::eventFilter(object, event);
}
