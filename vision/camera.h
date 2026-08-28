#pragma once

#include <librealsense2/hpp/rs_processing.hpp>
#include <librealsense2/hpp/rs_sensor.hpp>
#include <librealsense2/rs.hpp>
#include <opencv4/opencv2/core.hpp>

namespace vision {

struct CameraFrame {
    cv::Mat color;
    cv::Mat depth;
    double timestamp;
};

class Camera {
   public:
    Camera();

    void init(int width, int height, int fps);

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    CameraFrame wait_for_frame(unsigned timeout_ms = RS2_DEFAULT_TIMEOUT);

    void set_option(rs2_option, float);
    std::optional<float> get_option(rs2_option) const;

    std::optional<float> get_exposure() const;
    void set_exposure(float exposure);

   private:
    int _width = 0;
    int _height = 0;
    int _fps = 0;

    mutable rs2::pipeline _pipeline;
    rs2::pipeline_profile _profile;
    std::optional<rs2::sensor> _color_sensor;
    std::optional<rs2::depth_sensor> _depth_sensor;
    rs2::align _align_to_color;
    rs2::colorizer _colorizer;
    float _depth_scale = 0.01f;
};

}  // namespace vision
