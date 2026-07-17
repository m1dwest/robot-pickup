#include "main_view.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <plog/Log.h>
#include <string>

#include "../state.h"
#include "../utils.h"

namespace app {

void MainView::on_enter() {
    //
}

void MainView::update(app::State& state) {
    _viewport.set_frame(state.camera_frame);

    static const ImU32 polygon_color = IM_COL32(0, 255, 0, 255);
    static const ImU32 label_color = IM_COL32(255, 0, 0, 255);

    _viewport.clear_overlay();
    for (const auto& detection : state.aruco_detections) {
        _viewport.add_overlay_polygon({.points = detection.corners,
                                       .color = polygon_color,
                                       .thickness = 2,
                                       .closed = true});

        const auto polygon_center = utils::centroid(detection.corners);
        _viewport.add_overlay_label({.pos = polygon_center,
                                     .text = std::to_string(detection.id),
                                     .color = label_color});
    }
}

void MainView::compose() {
    ImGuiWindowFlags root_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    const auto root = ImGui::Begin("##root", nullptr, root_flags);
    ImGui::PopStyleVar(2);

    _viewport.compose(ImVec2{960, 400});
    compose_frame_scale();
}

void MainView::compose_frame_scale() {
    ImGui::SetNextItemWidth(80.0f);
    if (ImGui::BeginCombo(
            "Preview scale",
            _frame_scale_items.at(_frame_scale_id).first.c_str())) {
        for (int n = 0; n < _frame_scale_items.size(); ++n) {
            const bool is_selected = (_frame_scale_id == n);
            if (ImGui::Selectable(_frame_scale_items.at(n).first.c_str(),
                                  is_selected)) {
                _frame_scale_id = n;
                _viewport.set_scale(
                    _frame_scale_items.at(_frame_scale_id).second);
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
}

}  // namespace app
