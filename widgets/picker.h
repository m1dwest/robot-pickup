
#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <opencv2/opencv.hpp>

#include "widget.h"

namespace widget {

class Picker : public Widget {
   public:
    void compose() override;

    bool is_enabled() const;
    void set_pos_l(const ImVec2& pos);
    void set_pos_r(const ImVec2& pos);
    void set_depth_l(float depth);
    void set_depth_r(float depth);
    void set_projected_point_l(const cv::Point3f& point);
    void set_projected_point_r(const cv::Point3f& point);

   private:
    std::optional<float> calculate_distance() const;

    ImVec2 _size = ImVec2{360, 240};
    bool _is_enabled = false;

    ImVec2 _pos_l;
    ImVec2 _pos_r;
    float _depth_l;
    float _depth_r;
    cv::Point3f _point_l;
    cv::Point3f _point_r;
};

}  // namespace widget
