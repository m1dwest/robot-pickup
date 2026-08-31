#include "main_view.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <plog/Log.h>
#include <string>

#include "../utils.h"
#include "../widgets/guards.h"

namespace app {

void MainView::on_enter() {
    //
}

void MainView::update(app::State& state) {
    _viewport.set_frame(_selected_stream == SelectedStream::Color
                            ? state.camera_frame.get_color_rgb()
                            : state.camera_frame.get_depth_rgb());

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

    if (_picker.is_enabled()) {
        const auto depth = _viewport.get_clicked_frame_pos().transform(
            [&state](const auto& pos) {
                return state.camera_frame.get_distance(std::round(pos.x),
                                                       std::round(pos.y));
            });
        // TODO calculate depth only when coordinates has changed
        _picker.set_depth(depth);
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

    _viewport.compose(_viewport_size);
    ImGui::SameLine();
    if (_picker.is_enabled()) {
        _picker.set_clicked_pos(_viewport.get_clicked_frame_pos());
    }
    _picker.compose();
    compose_viewport_control();
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
}

void MainView::compose_stream_combo() {
    ImGui::SetNextItemWidth(80.0f);
    if (int selected_stream = static_cast<int>(_selected_stream);
        ImGui::BeginCombo("Stream",
                          _stream_items.at(selected_stream).c_str())) {
        for (int n = 0; n < _stream_items.size(); ++n) {
            const bool is_selected = (selected_stream == n);
            if (ImGui::Selectable(_stream_items.at(n).c_str(), is_selected)) {
                _selected_stream = static_cast<SelectedStream>(n);
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void MainView::compose_viewport_control() {
    const auto _ = widget::ImGuiChildScope(
        "##viewport_control", ImVec2(_viewport_size.x, 40),
        ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    compose_frame_scale();
    ImGui::SameLine();
    compose_stream_combo();
}

}  // namespace app
