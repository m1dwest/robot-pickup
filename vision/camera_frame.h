#pragma once

#include <librealsense2/rs.hpp>
#include <opencv4/opencv2/core.hpp>

namespace vision {

struct CameraFrame {
   public:
    double timestamp = 0;

    CameraFrame();
    CameraFrame(rs2::video_frame color, rs2::depth_frame depth,
                double timestamp);

    cv::Mat get_color_rgb() const;
    cv::Mat get_depth_rgb() const;

    float get_depth(int x, int y) const;
    cv::Point3f deproject_point(int x, int y) const;
    cv::Point3f deproject_point(int x, int y, float depth) const;

    cv::Point3f get_deprojected_center(const std::vector<cv::Point2f>& vertices,
                                       int stride) const;

   private:
    struct DepthGeometry {
        std::vector<cv::Point3f> points;
        cv::Vec3f center_ray;
    };

    DepthGeometry collect_3d_points(const std::vector<cv::Point2f>& vertices,
                                    int stride) const;

    rs2::video_frame _color;
    rs2::depth_frame _depth;
    mutable std::optional<cv::Mat> _color_rgb;
    mutable std::optional<cv::Mat> _depth_rgb;
    mutable std::optional<rs2_intrinsics> _intrinsics;

    rs2::colorizer _colorizer;
};

}  // namespace vision
