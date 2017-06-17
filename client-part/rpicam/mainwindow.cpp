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

 //       printf("recvf = %d\n", f);
        g_recv_frame = f;
   //     printf("f = %d\n", f);
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

void run2()
{
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());

    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());
    g_mat_queue.push_back(FrameObj());

    pthread_t th_id;
    pthread_create(&th_id, NULL, reccFrame, NULL);

  //  cv::namedWindow( "vv", cv::WINDOW_AUTOSIZE );
    for (int f = 0; f < 5000; ++f)
    {

        printf(">>> f = %d\n", f);
        int idx = f % MAT_BUF_NUM;
        while (g_mat_queue[idx].is_ok == 0)
        {
            printf("oo");

        }


        if (g_wait_show == 0 && (g_recv_frame - f) < 5)
        {
            cv::imshow("vv", g_mat_queue[idx].mat_buf);
     //       cv::imshow("v2", g_mat_queue[idx].mat_buf_post);
        }
        else
            printf("skip! %d, %d\n", g_recv_frame, f);

        g_mat_queue[idx].is_ok = 0;

        waitKey(1);
    }

  //  pclose(fp);

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

        static int r = 0;
        qDebug() << "timer" << ++r;
    }

}

void MainWindow::on_pushButton_clicked()
{
    {
        if (g_mat_queue.size() == 0)
        {
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());

            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
            g_mat_queue.push_back(FrameObj());
        }

        pthread_t th_id;
        pthread_create(&th_id, NULL, reccFrame, NULL);


        cv::namedWindow( "vv", cv::WINDOW_AUTOSIZE );
        for (int f = 0; f < 5000; ++f)
        {

            int idx = f % MAT_BUF_NUM;
            while (g_mat_queue[idx].is_ok == 0)
            {

            }


            if (g_wait_show == 0 && (g_recv_frame - f) < 5)
            {
                cv::imshow("vv", g_mat_queue[idx].mat_buf);
                cv::imshow("v2", g_mat_queue[idx].mat_buf_post);
            }
            else
                printf("skip! %d, %d\n", g_recv_frame, f);

            g_mat_queue[idx].is_ok = 0;

            waitKey(1);
        }


        return ;
    }
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

#if 0



#endif

