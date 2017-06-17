#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <pthread.h>
#include <vector>
#include <iostream>
#include <unistd.h>

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

static
QImage Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp(src.cols,src.rows,src.type());
     cvtColor(src, temp,CV_BGR2RGB);
     QImage dest= QImage((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     return dest;
}

void MainWindow::readFrame(void)
{
    using namespace cv;
    //for (int i = 0; i < 500; ++i)
    {
        Mat frame;
        if (!m_cam.read(frame))
            return;

     //   imshow("test", frame);
      //  if (waitKey(10) == 'q')
     //       break;

       // usleep(1000*50);
        QImage  image = Mat2QImage(frame);

        ui->label->setPixmap(QPixmap::fromImage(image));
        ui->label->update();

    }

}

void MainWindow::on_pushButton_clicked()
{
    using namespace cv;

    //cv::VideoCapture cam("/tmp/x.mov");

    m_cam.open("/tmp/x.mov");

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
    timer->start(1);
#if 0
    for (int i = 0; i < 500; ++i)
    {
        Mat frame;
        if (!cam.read(frame))
            break;

     //   imshow("test", frame);
      //  if (waitKey(10) == 'q')
     //       break;

        usleep(1000*50);
        QImage  image = Mat2QImage(frame);

     //   ui->label->setPixmap(QPixmap::fromImage(image));
     //   ui->label->update();

    }
#endif

    //cam.release();
}
