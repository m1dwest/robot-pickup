#include "camera.h"

#include <librealsense2/h/rs_sensor.h>
#include <opencv2/core/hal/interface.h>
#include <plog/Log.h>
#include <librealsense2/hpp/rs_frame.hpp>
#include <librealsense2/hpp/rs_sensor.hpp>

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

float get_depth_scale(const std::optional<rs2::depth_sensor>& sensor) {
    if (sensor.has_value()) {
        return sensor.value().get_depth_scale();
    } else {
        LOG_WARNING << "Failed to get depth scale; defaulting to 0.001\n";
        return 0.001;
    }
}

}  // namespace

namespace vision {

Camera::Camera() : _align_to_color(RS2_STREAM_COLOR) {
    check_connected_cameras();
}

void Camera::init(int width, int height, int fps) {
    rs2::config cfg;
    cfg.disable_all_streams();
    cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, fps);
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);

    _profile = _pipeline.start(cfg);
    _color_sensor = get_sensor<rs2::color_sensor>(_profile);
    _depth_sensor = get_sensor<rs2::depth_sensor>(_profile);
    _depth_scale = get_depth_scale(_depth_sensor);

    _width = width;
    _height = height;
    _fps = fps;
}

CameraFrame Camera::wait_for_frame(
    unsigned timeout_ms /*= RS2_DEFAULT_TIMEOUT*/) {
    const auto frames = _pipeline.wait_for_frames(timeout_ms);
    const auto aligned_frames = _align_to_color.process(frames);
    auto color = aligned_frames.get_color_frame();
    auto depth = aligned_frames.get_depth_frame();
    double ts_ms = color.get_timestamp();

    return CameraFrame{color, depth, ts_ms};
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
