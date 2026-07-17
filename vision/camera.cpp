#include "camera.h"

#include <plog/Log.h>

namespace {

template <typename T>
std::optional<T> get_sensor(const rs2::pipeline_profile& profile) {
    try {
        return profile.get_device().first<T>();
    } catch (rs2::error) {
        LOG_WARNING << "Failed to get sensor typeid:"
                    << std::string{typeid(T).name()};
        return std::nullopt;
    }
}

void check_connected_cameras() {
    rs2::context ctx;
    rs2::device_list devices = ctx.query_devices();
    if (devices.size() == 0) {
        throw std::runtime_error{"No camera was found"};
    }

    for (const auto& d : devices) {
        LOG_DEBUG << "Camera found:";
        LOG_DEBUG << "Name: " << d.get_info(RS2_CAMERA_INFO_NAME)
                  << " | S/N: " << d.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)
                  << " | USB: "
                  << d.get_info(RS2_CAMERA_INFO_USB_TYPE_DESCRIPTOR);
    }
}

inline cv::Mat frame_to_mat(auto&& frame, int type) {
    return cv::Mat(frame.get_height(), frame.get_width(), type,
                   (void*)frame.get_data(), cv::Mat::AUTO_STEP);
}

}  // namespace

namespace vision {

Camera::Camera(int width, int height, int fps)
    : _width(width), _height(height), _fps(fps) {
    check_connected_cameras();

    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, fps);
    _profile = _pipeline.start(cfg);
    _color_sensor = get_sensor<rs2::color_sensor>(_profile);
}

Camera::~Camera() { _pipeline.stop(); }

cv::Mat Camera::wait_for_frame(
    unsigned timeout_ms /*= RS2_DEFAULT_TIMEOUT*/) const {
    const auto frames = _pipeline.wait_for_frames(timeout_ms);
    auto color = frames.get_color_frame();
    double ts_ms = color.get_timestamp();

    if (!color) {
        LOG_WARNING << "Couldn't get color frame";
        return cv::Mat(_width, _height, CV_8UC3);
    }

    return frame_to_mat(std::move(color), CV_8UC3);
}

void Camera::set_option(rs2_option option, float value) {
    if (_color_sensor.has_value()) {
        try {
            _color_sensor.value().set_option(option, value);
        } catch (const rs2::error& e) {
            LOG_ERROR << std::format("Failed to set {}: {}",
                                     rs2_option_to_string(option), e.what());
        }
    } else {
        LOG_ERROR << std::format(
            "Failed to set {}. No valid depth sensor was found",
            rs2_option_to_string(option));
    }
}

std::optional<float> Camera::get_option(rs2_option option) const {
    if (_color_sensor.has_value()) {
        try {
            return _color_sensor.value().get_option(option);
        } catch (const rs2::error& e) {
            LOG_ERROR << std::format("Failed to get {}: {}",
                                     std::string{rs2_option_to_string(option)},
                                     e.what());
        }
    } else {
        LOG_ERROR << std::format(
            "Failed to get {}. No valid depth sensor was found",
            rs2_option_to_string(option));
    }

    return std::nullopt;
}

void Camera::set_exposure(float exposure) {
    set_option(RS2_OPTION_EXPOSURE, exposure);
}

std::optional<float> Camera::get_exposure() const {
    return get_option(RS2_OPTION_EXPOSURE);
}

}  // namespace vision
