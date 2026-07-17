#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <opencv2/opencv.hpp>

#include "widget.h"

namespace widget {

class Viewport : public Widget {
   public:
    struct OverlayPolygon {
        std::vector<cv::Point2f> points;
        ImU32 color;
        float thickness;
        bool closed = true;
    };

    struct OverlayLabel {
        cv::Point2f pos;
        std::string text;
        ImU32 color;
    };

    void set_frame(const cv::Mat& frame);
    void set_scale(float scale);

    void clear_overlay();
    void add_overlay_polygon(OverlayPolygon&& polygon);
    void add_overlay_label(OverlayLabel&& label);

    void compose() override;
    void compose(const ImVec2& size);

   private:
    float _frame_w;
    float _frame_h;
    unsigned _frame_tex = 0;

    float _scale;

    std::vector<OverlayPolygon> _overlay_polygons;
    std::vector<OverlayLabel> _overlay_labels;
};

}  // namespace widget
