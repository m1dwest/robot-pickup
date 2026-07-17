#pragma once

#include <opencv2/core.hpp>

#include "vision/aruco.h"

namespace app {

struct State {
    cv::Mat camera_frame;
    std::vector<vision::Aruco::Detection> aruco_detections;
};

}  // namespace app
