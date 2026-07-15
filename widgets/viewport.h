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
    void set_frame(const cv::Mat& frame);
    void set_scale(float scale);
    void compose() override;

   private:
    struct FrameGeometry {
        ImVec2 pos;
        ImVec2 size;
        ImVec2 uv_0;
        ImVec2 uv_1;
    };

    FrameGeometry calc_frame_geometry(const ImVec2& available,
                                      const ImVec2& frame_pos);

    int _w;
    int _h;
    unsigned _tex = 0;

    float _scale;
};

}  // namespace widget
