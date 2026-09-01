#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>

#include "widget.h"

namespace widget {

class Viewport : public Widget {
    enum ClickedButton { Left = 0, Right = 1 };

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

    struct OverlayPoint {
        cv::Point2f pos;
        float radius;
        ImU32 color;
    };

    struct OverlayLine {
        cv::Point2f p_1;
        cv::Point2f p_2;
        float thickness;
        ImU32 color;
    };

    void set_frame(const cv::Mat& frame);
    void set_scale(float scale);
    std::optional<ImVec2> get_clicked_pos_l() const;
    std::optional<ImVec2> get_clicked_pos_r() const;

    void clear_overlay();
    void add_overlay_polygon(OverlayPolygon polygon);
    void add_overlay_label(OverlayLabel label);
    void add_overlay_distance(OverlayPoint p1, OverlayPoint p2, float distance);

    void compose() override;
    void compose(const ImVec2& size);

   private:
    void update_clicked_pos(ClickedButton);

    float _frame_w;
    float _frame_h;
    unsigned _frame_tex = 0;
    std::optional<ImVec2> _clicked_pos_l, _clicked_pos_r;

    float _scale;
    float _true_scale;

    std::vector<OverlayPolygon> _overlay_polygons;
    std::vector<OverlayLabel> _overlay_labels;
    std::vector<OverlayPoint> _overlay_points;
    std::vector<OverlayLine> _overlay_lines;
    std::optional<OverlayPoint> _overlay_point_l, _overlay_point_r;
};

}  // namespace widget
