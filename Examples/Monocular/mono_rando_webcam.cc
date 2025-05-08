#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <System.h>

using namespace std;

bool b_continue_session;

void exit_loop_handler(int s) {
    cout << "Finishing session" << endl;
    b_continue_session = false;
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 5) {
        cerr << endl << "Usage: ./mono_webcam path_to_vocabulary path_to_settings camera_index  (trajectory_file_name)" << endl;
        return 1;
    }

    string file_name;
    bool bFileName = false;
    if (argc == 5) {
        file_name = string(argv[4]);
        bFileName = true;
    }

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = exit_loop_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    b_continue_session = true;

    // Open webcam
    cv::VideoCapture cap(std::stoi(argv[3])); // Default camera ID
    if (!cap.isOpened()) {
        cerr << "ERROR: Could not open webcam" << endl;
        return -1;
    }

    // Create SLAM system
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, true);
    float imageScale = SLAM.GetImageScale();

    cv::Mat imCV;
    double t_resize = 0.f;
    double t_track = 0.f;

    while (b_continue_session) {
        cap >> imCV;
        if (imCV.empty()) continue;

        double timestamp_ms = chrono::duration_cast<chrono::duration<double, std::milli>>(
            chrono::system_clock::now().time_since_epoch()
        ).count();

        if (imageScale != 1.f) {
#ifdef REGISTER_TIMES
    #ifdef COMPILEDWITHC11
            std::chrono::steady_clock::time_point t_Start_Resize = std::chrono::steady_clock::now();
    #else
            std::chrono::monotonic_clock::time_point t_Start_Resize = std::chrono::monotonic_clock::now();
    #endif
#endif
            int width = imCV.cols * imageScale;
            int height = imCV.rows * imageScale;
            cv::resize(imCV, imCV, cv::Size(width, height));
#ifdef REGISTER_TIMES
    #ifdef COMPILEDWITHC11
            std::chrono::steady_clock::time_point t_End_Resize = std::chrono::steady_clock::now();
    #else
            std::chrono::monotonic_clock::time_point t_End_Resize = std::chrono::monotonic_clock::now();
    #endif
            t_resize = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t_End_Resize - t_Start_Resize).count();
            SLAM.InsertResizeTime(t_resize);
#endif
        }

        // Convert to grayscale if needed
        if (imCV.channels() == 3) {
            cv::cvtColor(imCV, imCV, cv::COLOR_BGR2GRAY);
        }

#ifdef REGISTER_TIMES
  #ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
  #else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
  #endif
#endif

        SLAM.TrackMonocular(imCV, timestamp_ms);

#ifdef REGISTER_TIMES
  #ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
  #else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
  #endif
        t_track = t_resize + std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t2 - t1).count();
        SLAM.InsertTrackTime(t_track);
#endif
    }

    cap.release();
    SLAM.Shutdown();

    return 0;
}
