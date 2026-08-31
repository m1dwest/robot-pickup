
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
    void set_clicked_pos(const ImVec2& pos);
    void set_depth(float depth);
    void set_projected_point(const cv::Point3f& point);

   private:
    ImVec2 _size = ImVec2{360, 240};
    bool _is_enabled = false;

    ImVec2 _clicked_pos;
    float _depth;
    cv::Point3f _point;
};

}  // namespace widget
