#pragma once

#include <librealsense2/rs.hpp>
#include <opencv4/opencv2/core.hpp>

namespace vision {

class Camera {
   public:
    Camera(int width, int height, int fps);
    ~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    cv::Mat wait_for_frame(unsigned timeout_ms = RS2_DEFAULT_TIMEOUT) const;

    void set_option(rs2_option, float);
    std::optional<float> get_option(rs2_option) const;

    std::optional<float> get_exposure() const;
    void set_exposure(float exposure);

   private:
    int _width;
    int _height;
    int _fps;

    mutable rs2::pipeline _pipeline;
    rs2::pipeline_profile _profile;
    std::optional<rs2::sensor> _color_sensor;
};

}  // namespace vision
