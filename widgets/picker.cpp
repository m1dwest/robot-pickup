#include "picker.h"
#include <imgui.h>

#include <optional>
#include <string>

#include "guards.h"

namespace widget {

void Picker::compose() {
    const auto _ =
        widget::ImGuiChildScope("##picker", _size, ImGuiChildFlags_None,
                                ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SeparatorText("Picker");

    ImGui::Checkbox("Enable", &_is_enabled);

    ImGui::BeginDisabled(!_is_enabled);
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Frame surface point")) {
        const auto x = std::format(
            "X: {}",
            std::to_string(static_cast<int>(std::roundf(_clicked_pos.x))));
        ImGui::TextUnformatted(x.c_str());

        const auto y = std::format(
            "Y: {}",
            std::to_string(static_cast<int>(std::roundf(_clicked_pos.y))));
        ImGui::TextUnformatted(y.c_str());

        const auto depth = std::format("Depth: {:.4f}", _depth);
        ImGui::TextUnformatted(depth.c_str());
        ImGui::TreePop();
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Projected point")) {
        ImGui::TextUnformatted(std::format("X: {:.4f}", _point.x).c_str());
        ImGui::TextUnformatted(std::format("Y: {:.4f}", _point.y).c_str());
        ImGui::TextUnformatted(std::format("Z: {:.4f}", _point.z).c_str());
        ImGui::TreePop();
    }

    ImGui::EndDisabled();
}

bool Picker::is_enabled() const { return _is_enabled; }

void Picker::set_clicked_pos(const ImVec2& pos) { _clicked_pos = pos; }

void Picker::set_depth(float depth) { _depth = depth; }

void Picker::set_projected_point(const cv::Point3f& point) { _point = point; }

}  // namespace widget
