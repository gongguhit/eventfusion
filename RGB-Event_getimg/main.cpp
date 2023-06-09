#include <iostream>
#include <cstring>
#include <thread>
#include <algorithm>
#include "celex5.h"
#include<unistd.h>
#include <signal.h>
#include<pthread.h>
#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>
#include "example.hpp"
#include <opencv2/imgproc.hpp>
#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"


using namespace std;

using namespace cv;


CeleX5 *pCeleX5 = new CeleX5;

#define FPN_PATH    "/home/emrys/CLionProjects/Celex_view/FPN_2.txt"

uint8_t * pSensorBuffer = new uint8_t[CELEX5_PIXELS_NUMBER];

Mat RGB_IMG;



void exit_handler(int sig_num)
{
    printf("SIGNAL received: num =%d\n", sig_num);
    if (sig_num == 1 || sig_num == 2 || sig_num == 3 || sig_num == 9 || sig_num == 15)
    {
        delete pCeleX5;
        pCeleX5 = NULL;
        exit(0);
    }
}



Mat Circle_det = Mat::ones(480,640,CV_8U);



Mat detectCircle(Mat &img )
{
    Mat grayColor;

    cvtColor(img,grayColor, COLOR_BGR2GRAY);

    SimpleBlobDetector::Params params;

    params.maxArea = 500;
    params.minArea = 40;
    params.minDistBetweenBlobs = 5;

    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

    std::vector<cv::Point2f> p;
    std::vector<cv::Point2f> corners;
    cv::Size s;
    s.height = 8;
    s.width = 12;

//    try {
        bool found = cv::findCirclesGrid(grayColor, s, corners, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, detector);

        if (!found) {
//            cout << "Could not find chess board!" << endl;
            return grayColor;

        }

        cv::drawChessboardCorners(img, s, corners, found);
        return img;
    }
//    catch (const char* msg) {
//            cerr << msg << endl;}

Mat detect_Event(Mat img_event)
{
    SimpleBlobDetector::Params params;

    params.maxArea = 500;
    params.minArea = 40;
    params.minDistBetweenBlobs = 5;

    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

    std::vector<cv::Point2f> p;
    std::vector<cv::Point2f> corners;
    cv::Size s;
    s.height = 8;
    s.width = 12;

//    try {
    bool found = cv::findCirclesGrid(img_event, s, corners, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, detector);

    if (!found) {
            cout << "Could not find chess board!" << endl;


    }

    cv::drawChessboardCorners(img_event, s, corners, found);
    return img_event;
}

int main()
{

    pCeleX5->openSensor(CeleX5::CeleX5_MIPI);
    pCeleX5->setFpnFile(FPN_PATH);
    pCeleX5->disableEventStreamModule();
    CeleX5::CeleX5Mode sensorMode = CeleX5::Full_Picture_Mode;
//    CeleX5::CeleX5Mode sensorMode = CeleX5::Event_Off_Pixel_Timestamp_Mode;
    pCeleX5->setSensorFixedMode(sensorMode);

    // install signal use sigaction
    struct sigaction sig_action;
    sigemptyset(&sig_action.sa_mask);
    sig_action.sa_flags = 0;
    sig_action.sa_handler = exit_handler;
    sigaction(SIGHUP, &sig_action, NULL);  // 1
    sigaction(SIGINT, &sig_action, NULL);  // 2
    sigaction(SIGQUIT, &sig_action, NULL); // 3
    sigaction(SIGKILL, &sig_action, NULL); // 9
    sigaction(SIGTERM, &sig_action, NULL); // 15

    Mat matFullPic(800, 1280, CV_8UC1, pSensorBuffer);
    Mat matFullPic_flipped;

    rs2::pipeline pipe;
    pipe.start();


//    while (waitKey(1) < 0 && getWindowProperty(window_name, WND_PROP_AUTOSIZE) >= 0) {
    while (true)
        {

            rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
            //rs2::frame depth = data.get_depth_frame().apply_filter(color_map);
            rs2::frame color = data.get_color_frame();

            // Query frame size (width and height)
            //const int w = depth.as<rs2::video_frame>().get_width();
            const int w = color.as<rs2::video_frame>().get_width();
            //const int h = depth.as<rs2::video_frame>().get_height();
            const int h = color.as<rs2::video_frame>().get_height();

            // Create OpenCV matrix of size (w,h) from the colorized depth data
            //Mat image(Size(w, h), CV_8UC3, (void*)depth.get_data(), Mat::AUTO_STEP);
            Mat image(Size(w, h), CV_8UC3, (void *) color.get_data(), Mat::AUTO_STEP);
            //Dontla 20210827

            cvtColor(image, RGB_IMG, CV_BGR2RGB);//转换

            const auto window_name = "RGB_IMG";
            namedWindow(window_name, WINDOW_AUTOSIZE);

            Mat rst;
            rst = detectCircle(RGB_IMG);


            imshow(window_name, rst);


            pCeleX5->getFullPicBuffer(pSensorBuffer);

            flip(matFullPic,matFullPic_flipped,1);

            Mat resized_eventimg;

            int resized_width = 640;
            int resized_height = 400;

            resize(matFullPic_flipped, resized_eventimg, Size(resized_width, resized_height));


            imshow("Event_resized", detect_Event(resized_eventimg));

            waitKey(30);


        }

}











//
//int get_depth()
//{
//    // Create a Pipeline - this serves as a top-level API for streaming and processing frames
//    rs2::pipeline p;
//    // Configure and start the pipeline
//    p.start();
//    // Block program until frames arrive
//    rs2::frameset frames = p.wait_for_frames();
//    // Try to get a frame of a depth image
//    rs2::depth_frame depth = frames.get_depth_frame();
//    // Get the depth frame's dimensions
//    float width = depth.get_width();
//    float height = depth.get_height();
//    // Query the distance from the camera to the object in the center of the image
//    float dist_to_center = depth.get_distance(width / 2, height / 2);
//    // Print the distance
//    std::cout << "The camera is facing an object " << dist_to_center << " meters away \r";
//    return 0;
//}