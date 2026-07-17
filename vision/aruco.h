#pragma once

#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/opencv.hpp>

namespace vision {

class Aruco {
   public:
    struct Detection {
        int id;
        std::vector<cv::Point2f> corners;
    };

    Aruco();

    void detect(const cv::Mat& image);

    std::vector<cv::Point2f> corners(int id) const;
    std::vector<int> ids() const;
    std::vector<Detection> detections() const;

   private:
    cv::aruco::ArucoDetector _detector;

    std::vector<int> _ids;
    std::vector<std::vector<cv::Point2f>> _corners;
    std::vector<std::vector<cv::Point2f>> _rejected;
};

}  // namespace vision
