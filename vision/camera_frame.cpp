#include "camera_frame.h"

#include <plog/Log.h>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>

#include "../utils.h"

namespace {

inline cv::Mat frame_to_mat(const rs2::video_frame& frame, int type) {
    return cv::Mat(frame.get_height(), frame.get_width(), type,
                   (void*)frame.get_data(), cv::Mat::AUTO_STEP);
}

cv::Mat create_mask(const std::vector<cv::Point2f>& vertices, int w, int h) {
    std::vector<cv::Point> polygon;
    polygon.reserve(vertices.size());

    for (const auto& p : vertices)
        polygon.emplace_back(cvRound(p.x), cvRound(p.y));

    cv::Mat mask(h, w, CV_8UC1, cv::Scalar(0));
    cv::fillConvexPoly(mask, polygon, 255);

    return mask;
}

cv::Point2f line_intersection(const cv::Point2f& a, const cv::Point2f& b,
                              const cv::Point2f& c, const cv::Point2f& d) {
    cv::Point2f r = b - a;
    cv::Point2f s = d - c;

    float cross = r.x * s.y - r.y * s.x;

    if (std::abs(cross) < 1e-6f) {
        return {};
    }

    cv::Point2f ca = c - a;
    float t = (ca.x * s.y - ca.y * s.x) / cross;

    return a + t * r;
}

}  // namespace

namespace vision {

CameraFrame::CameraFrame() : _color(rs2::frame{}), _depth(rs2::frame{}) {}

CameraFrame::CameraFrame(rs2::video_frame color, rs2::depth_frame depth,
                         double timestamp)
    : _color(std::move(color)),
      _depth(std::move(depth)),
      timestamp(timestamp) {}

cv::Mat CameraFrame::get_color_rgb() const {
    if (!_color_rgb.has_value()) {
        if (!_color) {
            LOG_WARNING << "Couldn't get color frame";
            _color_rgb =
                cv::Mat(_color.get_width(), _color.get_height(), CV_8UC3);
        } else {
            _color_rgb = frame_to_mat(_color, CV_8UC3);
        }
    }
    return _color_rgb.value();
}

cv::Mat CameraFrame::get_depth_rgb() const {
    if (!_depth_rgb.has_value()) {
        if (!_depth) {
            LOG_WARNING << "Couldn't get depth frame";
            _depth_rgb =
                cv::Mat(_depth.get_width(), _depth.get_height(), CV_8UC3);
        } else {
            auto depth_rgb = _colorizer.process(_depth);
            _depth_rgb = frame_to_mat(depth_rgb, CV_8UC3);
        }
    }
    return _depth_rgb.value();
}

float CameraFrame::get_depth(int x, int y) const {
    return _depth.get_distance(x, y);
}

cv::Point3f CameraFrame::deproject_point(int x, int y) const {
    const auto depth = get_depth(x, y);
    return deproject_point(x, y, depth);
}

cv::Point3f CameraFrame::deproject_point(int x, int y, float depth) const {
    if (!_intrinsics.has_value()) {
        _intrinsics = _depth.get_profile()
                          .as<rs2::video_stream_profile>()
                          .get_intrinsics();
    }

    const float pixel[2] = {static_cast<float>(x), static_cast<float>(y)};

    cv::Point3f point;
    rs2_deproject_pixel_to_point(reinterpret_cast<float*>(&point),
                                 &_intrinsics.value(), pixel, depth);
    return point;
}

CameraFrame::DepthGeometry CameraFrame::collect_3d_points(
    const std::vector<cv::Point2f>& vertices, int stride = 1) const {
    const auto w = _depth.get_width();
    const auto h = _depth.get_height();
    const auto mask = create_mask(vertices, h, w);

    const auto center =
        line_intersection(vertices[0], vertices[2], vertices[1], vertices[3]);
    const auto center_ray = deproject_point(center.x, center.y, 1.0f);

    std::vector<cv::Point3f> points;

    for (int y = 0; y < h; y += stride) {
        for (int x = 0; x < w; x += stride) {
            if (!mask.at<uint8_t>(y, x)) {
                continue;
            }

            float depth = get_depth(x, y);

            if (depth <= 0.0f) {
                continue;
            }

            points.push_back(deproject_point(x, y));
        }
    }

    return DepthGeometry{
        .points = points,
        .center_ray = center_ray,
    };
}

cv::Point3f CameraFrame::get_deprojected_center(
    const std::vector<cv::Point2f>& vertices, int stride = 1) const {
    const auto [points, center_ray] = collect_3d_points(vertices, stride);

    cv::Point3f centroid{0, 0, 0};

    for (const auto& p : points) {
        centroid.x += p.x;
        centroid.y += p.y;
        centroid.z += p.z;
    }

    const auto points_n = points.size();
    centroid *= 1.0f / points_n;

    cv::Mat A(points_n, 3, CV_32F);

    for (size_t i = 0; i < points_n; ++i) {
        A.at<float>(i, 0) = points[i].x - centroid.x;
        A.at<float>(i, 1) = points[i].y - centroid.y;
        A.at<float>(i, 2) = points[i].z - centroid.z;
    }

    cv::SVD svd(A, cv::SVD::FULL_UV);

    cv::Vec3f normal(svd.vt.at<float>(2, 0), svd.vt.at<float>(2, 1),
                     svd.vt.at<float>(2, 2));

    float d = -normal.dot(cv::Vec3f(centroid.x, centroid.y, centroid.z));

    float t = -d / normal.dot(center_ray);
    cv::Vec3f centerPoint = t * center_ray;

    return {centerPoint};
}

}  // namespace vision
