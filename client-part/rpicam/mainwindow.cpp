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
#include <QDebug>

using namespace cv;
using namespace std;

#define MAT_BUF_NUM 10

struct FrameObj
{
public:
    FrameObj(void): is_ok(0)
    {
    }

    Mat mat_buf;
    Mat mat_buf_post;
    int is_ok;

};

FILE * getCommandFp(void)
{
    const char * cmd =
        "/usr/local/bin/ffmpeg -i /tmp/fifo  -pix_fmt bgr24 -vcodec rawvideo -an -sn -f image2pipe -";

  //  const char * cmd = "ls";
    errno = 0;
    FILE * fp = popen(cmd, "r");

    printf("fp = %p\n", fp);
    perror("");
    return fp;

}

vector<FrameObj> g_mat_queue;
int g_wait_show = 0;
int g_recv_frame = 0;


void * reccFrame(void*)
{

    FILE * fp = getCommandFp();

    int N = 640*480*3;
    int dim = 2;
    int dims[2] = {480, 640};

    uint8_t * buf = new uint8_t[640*480*3];
    if (!buf)
    {
        fprintf(stderr, "3434\n");
        exit(0);

    }

    memset(buf, 0, N);

    uint8_t * pbuf = buf;
    for (int f = 0; f < 5000; ++f)
    {
#if 0
        qDebug(">> %lu, (%2d) %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n",
               g_mat_queue.size(),
               f % MAT_BUF_NUM,
               g_mat_queue[0].is_ok,
               g_mat_queue[1].is_ok,
               g_mat_queue[2].is_ok,
               g_mat_queue[3].is_ok,
               g_mat_queue[4].is_ok,
               g_mat_queue[5].is_ok,
               g_mat_queue[6].is_ok,
               g_mat_queue[7].is_ok,
               g_mat_queue[8].is_ok,
               g_mat_queue[9].is_ok);
#endif

      //  qDebug() << "recv f = " << f;
        g_recv_frame = f;
        int idx = f % MAT_BUF_NUM;
        g_wait_show = 0;
        while (g_mat_queue[idx].is_ok > 0)
        {
            g_wait_show = 1;
        //    printf("wait show idx = %1d\n", idx);
        }
        g_wait_show = 0;

        int m = N;
        pbuf = buf;

        while (1)
        {
            int ret = fread(pbuf, 1, m, fp);
           // printf("ret = %d\n", ret);
            if (ret <= 0)
            {
                fprintf(stderr, "orz! ret = %d\n", ret);
 //               f = 500000;
  //              break;
                continue;
            }

            // printf("ret ==== %d\n", ret);

            pbuf += ret;
            m -= ret;

            if (m <= 0)
                break;
        }

       // g_mat_queue[idx].mat_buf = cv::Mat(dim, dims, CV_8UC3, buf);
        Mat src = cv::Mat(dim, dims, CV_8UC3, buf);
        g_mat_queue[idx].mat_buf = src;

        if (0)
        {
            Mat gray_image;
            cvtColor(src, gray_image, CV_BGR2GRAY );
            g_mat_queue[idx].mat_buf = src;
            g_mat_queue[idx].mat_buf_post =gray_image;
        }

        g_mat_queue[idx].is_ok = 1;

    }

    return NULL;

}

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

static QImage Mat2QImage(cv::Mat const& src)
{
     cv::Mat temp(src.cols,src.rows,src.type());
     cvtColor(src, temp,CV_BGR2RGB);
     QImage dest= QImage((uchar*) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     return dest;
}

void MainWindow::readFrame(void)
{
    using namespace cv;

    {
        Mat frame;
        if (!m_cam.read(frame))
            return;

        QImage  image = Mat2QImage(frame);

        ui->label->setPixmap(QPixmap::fromImage(image));
        ui->label->update();

        static int r = 0;
        qDebug() << "timer" << ++r;
    }

}

void MainWindow::readFrame2(void)
{
    using namespace cv;
    static int f = 0;

    // qDebug() << "show f = " << f;

    int idx = f % MAT_BUF_NUM;
    ++f;
    while (g_mat_queue[idx].is_ok == 0)
    {
        //qDebug() << "wait for recv..";
    }

    QImage  image;

    if (g_wait_show == 0 && (g_recv_frame - f) < 9)
    {
        image = Mat2QImage(g_mat_queue[idx].mat_buf);
        ui->label->setPixmap(QPixmap::fromImage(image));
        ui->label->update();
    }
    else
    {
        qDebug("skip! gwait = %1d, %d, %d\n",
               g_wait_show, g_recv_frame, f);

        g_mat_queue[idx].is_ok = 0;
        return;
    }
    g_mat_queue[idx].is_ok = 0;

    usleep(1000);

}

void MainWindow::on_pushButton_clicked()
{
    if (g_mat_queue.size() == 0)
    {
        for (int i = 0; i < MAT_BUF_NUM; ++i)
        {
            g_mat_queue.push_back(FrameObj());
        }
    }

    pthread_t th_id;
    pthread_create(&th_id, NULL, reccFrame, NULL);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readFrame2()));
    timer->start(1);

    return;
}
