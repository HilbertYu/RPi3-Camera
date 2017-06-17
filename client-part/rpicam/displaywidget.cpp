#include "displaywidget.h"
#include "ui_displaywidget.h"

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


displayWidget::displayWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::displayWidget)
{
    ui->setupUi(this);
}

displayWidget::~displayWidget()
{
    delete ui;
}

static QImage Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp(src.cols,src.rows,src.type());
     cvtColor(src, temp,CV_BGR2RGB);
     QImage dest= QImage((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     return dest;
}

void displayWidget::run(void)
{

    using namespace cv;

    cv::VideoCapture cam("/tmp/x.mov");

#if 1
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

        ui->label->setPixmap(QPixmap::fromImage(image));
        update();

    }
#endif

    cam.release();
}
