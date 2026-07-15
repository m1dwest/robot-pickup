#include "camera.h"

#include <plog/Log.h>

namespace {

// inline cv::Mat frame_to_mat(auto&& frame, int type) {
//     return cv::Mat(frame.get_height(), frame.get_width(), type,
//                    (void*)frame.get_data());
// }
//
// float get_depth_scale(const std::optional<rs2::depth_sensor>& sensor) {
//     if (sensor.has_value()) {
//         return sensor.value().get_depth_scale();
//     } else {
//         LOG_WARNING << "Failed to get depth scale; defaulting to 0.001\n";
//         return 0.001;
//     }
// }

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

// cv::Mat rotate_image(cv::Mat&& mat, float angle) {
//     const auto center = cv::Point2f(static_cast<float>(mat.cols) / 2.0f,
//                                     static_cast<float>(mat.rows) / 2.0f);
//     const auto scale = 1.0f;
//     cv::Mat rotation_mat = cv::getRotationMatrix2D(center, angle, scale);
//
//     cv::Mat rotated;
//     cv::warpAffine(mat, rotated, rotation_mat, mat.size());
//
//     return rotated;
// }
//
// void render_grid(cv::Mat& image, const cv::Scalar color, int size, int
// offset_x,
//                  int offset_y) {
//     static const auto render_line = [&image, &color, thickness = 1,
//                                      line_type = cv::LINE_AA](
//                                         int x_1, int y_1, int x_2, int y_2) {
//         cv::Point p_1{x_1, y_1};
//         cv::Point p_2{x_2, y_2};
//         cv::line(image, p_1, p_2, color, thickness, line_type);
//     };
//
//     static const auto min_offset = [](int size, int offset) -> int {
//         if (offset < size) {
//             return offset;
//         } else {
//             int x = offset / size;
//             return offset % x;
//         }
//     };
//
//     const auto min_offset_x = min_offset(size, offset_x);
//     const auto min_offset_y = min_offset(size, offset_y);
//
//     for (auto x = size - min_offset_x; x < image.cols; x += size) {
//         render_line(x, 0, x, image.rows);
//     }
//
//     for (auto y = size - min_offset_y; y < image.rows; y += size) {
//         render_line(0, y, image.cols, y);
//     }
// }
//
// void render_centerline(cv::Mat& image, const cv::Scalar color, int offset) {
//     const auto thickness = 2;
//     const auto line_type = cv::LINE_AA;
//
//     cv::Point p_1{0, image.rows / 2 + offset};
//     cv::Point p_2{image.cols, image.rows / 2 + offset};
//
//     cv::line(image, p_1, p_2, color, thickness, line_type);
// }
inline cv::Mat frame_to_mat(auto&& frame, int type) {
    return cv::Mat(frame.get_height(), frame.get_width(), type,
                   (void*)frame.get_data());
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
