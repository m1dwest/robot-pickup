#pragma once

#include <opencv2/core.hpp>

#include "vision/aruco.h"

namespace app {

enum SelectedStream { Color, Depth };

struct State {
    SelectedStream selected_stream = SelectedStream::Color;
    cv::Mat camera_frame;
    std::vector<vision::Aruco::Detection> aruco_detections;
};

}  // namespace app
