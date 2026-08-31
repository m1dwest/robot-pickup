#pragma once

#include <imgui.h>
#include <opencv2/core.hpp>

#include "vision/aruco.h"
#include "vision/camera.h"

namespace app {

enum SelectedStream { Color, Depth };

struct State {
    vision::CameraFrame camera_frame;
    std::vector<vision::Aruco::Detection> aruco_detections;
};

}  // namespace app
