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

using namespace cv;
using namespace std;


#define MAT_BUF_NUM 10
//#########################################

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

vector<FrameObj> g_mat_queue;

FILE * getCommandFp(void)
{
    const char * cmd =
        "ffmpeg -i fifo  -pix_fmt bgr24 -vcodec rawvideo -an -sn -f image2pipe -";

  //  const char * cmd = "ls";
    errno = 0;
    FILE * fp = popen(cmd, "r");

    printf("fp = %p\n", fp);
    perror("");
    return fp;

}

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
            printf("wait show idx = %1d\n", idx);
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

            Mat edge;
            Canny(gray_image, edge, 50, 210, 3);

            vector<vector<Point> > contours;
            vector<Vec4i> hierarchy;

            findContours(edge, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

            int max_idx =  -1;
            {

                double max_area = -1;

                for (int i = 0; i < contours.size(); ++i)
                {
                    double area = contourArea(contours[i]);
                    if (area > max_area)
                    {
                        max_area = area;
                        max_idx = i;
                    }

                }
            }

            RNG rng(12345);

            for(int i = 0; i<contours.size(); i++)
            {
                i = max_idx;

                Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0, 255), 255);
                drawContours(src, contours, i, color, 2, 8, hierarchy);

                Point2f center;
                float radius;

                minEnclosingCircle(contours[i], center, radius);

                circle(gray_image, center, radius, Scalar(255));
                printf(">>>> %g\n", radius);
#if 0
                void minEnclosingCircle(InputArray points,
                        Point2f& center,
                        float& radius)
#endif
                break;
            }

            g_mat_queue[idx].mat_buf = src;
            g_mat_queue[idx].mat_buf_post =gray_image;

        }

        if (0)
        {
            cv::Mat hsv_image;
            cv::cvtColor(src, hsv_image, cv::COLOR_BGR2HSV);
            cv::Mat lower_red_hue_range;
            cv::Mat upper_red_hue_range;
            cv::inRange(hsv_image, cv::Scalar(0, 70, 50), cv::Scalar(10, 255, 255), lower_red_hue_range);
            cv::inRange(hsv_image, cv::Scalar(170, 170, 50), cv::Scalar(180, 255, 255), upper_red_hue_range);


            g_mat_queue[idx].mat_buf = src;
            g_mat_queue[idx].mat_buf_post = lower_red_hue_range | upper_red_hue_range;
        }

        if (0)
        {
            cv::Mat bgr2hsvImg, hsv2skinImg;
            cv::cvtColor(src, bgr2hsvImg, cv::COLOR_BGR2HSV);
            cv::inRange(bgr2hsvImg, cv::Scalar(0,58,40), cv::Scalar(35,174,255), hsv2skinImg);

            g_mat_queue[idx].mat_buf = src;
            g_mat_queue[idx].mat_buf_post = hsv2skinImg;
        }

        if (0)
        {
            Mat redOnly;
            inRange(src, Scalar(0, 0, 0), Scalar(0, 0, 255), redOnly);

            g_mat_queue[idx].mat_buf = src;
            g_mat_queue[idx].mat_buf_post = redOnly;

        }

        if (0)
        {
            cv::Mat gray_image;
            cv::Mat bin_image;
            cvtColor(src, gray_image, CV_BGR2GRAY );

            threshold(gray_image, bin_image, 150, 255, THRESH_BINARY);

            std::vector<std::vector<cv::Point> > contours ;
            cv::findContours(bin_image , contours ,
                    CV_RETR_LIST , CV_CHAIN_APPROX_NONE) ;

            cv::Mat result(gray_image.size() , CV_8U , cv::Scalar(100)) ;

            result = gray_image;
            cv::drawContours(result , contours ,
                    -1 , cv::Scalar(255) , 3) ;

            g_mat_queue[idx].mat_buf = src;
            g_mat_queue[idx].mat_buf_post = result;

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


  //  pclose(fp);

}

int main()
{
    run2();
    return 0;

    cv::Mat frame;
    //cv::VideoCapture cap("./test.264");
    //cv::VideoCapture cap("./ooo.avi");
  //  cv::VideoCapture cap("./ooo.avi");

    cv::VideoCapture cap;
    cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'P', '4', 'V'));

    cap.open("./test.mp4");



    if(!cap.isOpened()){
        printf("oo\n");
        return -1;
    }

    cv::namedWindow( "window", cv::WINDOW_AUTOSIZE );
    while(true)
    {
        int ret = cap.read(frame);
        printf("ret = %d\n", ret);
        if(!ret)
            break;
        cv::Mat src = cv::Mat(frame);
        cv::imshow( "window",  src );

        cout << src.rows << endl;
        cout << src.cols << endl;
        cout << src.size().width << endl;
        cout << src.size().height << endl;
        cout << src;
        break;
        cv::waitKey(1);
    }
}

