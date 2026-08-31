#include "picker.h"
#include <imgui.h>

#include <string>

#include "guards.h"

namespace {

bool is_depth_valid(float depth, float epsilon = 1e-2f) {
    return (depth > 0) && (depth > epsilon);
}

}  // namespace

namespace widget {

void Picker::compose() {
    float child_width =
        (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) /
        2.0f;
    float child_height = 0.0f;

    const auto _ =
        widget::ImGuiChildScope("Picker", _size, ImGuiChildFlags_None,
                                ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::SeparatorText("Picker");

    ImGui::Checkbox("Enable", &_is_enabled);
    ImGui::BeginDisabled(!_is_enabled);
    if (ImGui::BeginTable("GridTable", 2, ImGuiTableFlags_SizingFixedSame)) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        {
            ImGui::TextUnformatted("L: Frame Surface");
            const auto x = std::format(
                "X: {}",
                std::to_string(static_cast<int>(std::roundf(_pos_l.x))));
            ImGui::TextUnformatted(x.c_str());

            const auto y = std::format(
                "Y: {}",
                std::to_string(static_cast<int>(std::roundf(_pos_l.y))));
            ImGui::TextUnformatted(y.c_str());

            const auto depth = std::format("Depth: {:.4f}", _depth_l);
            ImGui::TextUnformatted(depth.c_str());

            ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeight()));
            ImGui::TextUnformatted("L: Projected");
            ImGui::TextUnformatted(
                std::format("X: {:.4f}", _point_l.x).c_str());
            ImGui::TextUnformatted(
                std::format("Y: {:.4f}", _point_l.y).c_str());
            ImGui::TextUnformatted(
                std::format("Z: {:.4f}", _point_l.z).c_str());
            ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeight()));
        }

        ImGui::TableNextColumn();
        {
            ImGui::TextUnformatted("R: Frame Surface");
            const auto x = std::format(
                "X: {}",
                std::to_string(static_cast<int>(std::roundf(_pos_r.x))));
            ImGui::TextUnformatted(x.c_str());

            const auto y = std::format(
                "Y: {}",
                std::to_string(static_cast<int>(std::roundf(_pos_r.y))));
            ImGui::TextUnformatted(y.c_str());

            const auto depth = std::format("Depth: {:.4f}", _depth_r);
            ImGui::TextUnformatted(depth.c_str());

            ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeight()));
            ImGui::TextUnformatted("L: Projected");
            ImGui::TextUnformatted(
                std::format("X: {:.4f}", _point_r.x).c_str());
            ImGui::TextUnformatted(
                std::format("Y: {:.4f}", _point_r.y).c_str());
            ImGui::TextUnformatted(
                std::format("Z: {:.4f}", _point_r.z).c_str());
            ImGui::Dummy(ImVec2(0.0f, ImGui::GetTextLineHeight()));
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const auto distance =
            calculate_distance()
                .transform([](float value) {
                    return std::format("{:.4f} mm", value * 1000.0);
                })
                .value_or("NaN");
        ImGui::Text("Distance: %s", distance.c_str());

        ImGui::EndTable();
    }
    ImGui::EndDisabled();
}

bool Picker::is_enabled() const { return _is_enabled; }

void Picker::set_pos_l(const ImVec2& pos) { _pos_l = pos; }
void Picker::set_pos_r(const ImVec2& pos) { _pos_r = pos; }

void Picker::set_depth_l(float depth) { _depth_l = depth; }
void Picker::set_depth_r(float depth) { _depth_r = depth; }

void Picker::set_projected_point_l(const cv::Point3f& point) {
    _point_l = point;
}
void Picker::set_projected_point_r(const cv::Point3f& point) {
    _point_r = point;
}

std::optional<float> Picker::calculate_distance() const {
    if (is_depth_valid(_depth_l) && is_depth_valid(_depth_r)) {
        return static_cast<float>(std::abs(cv::norm(_point_l - _point_r)));
    }

    return std::nullopt;
}

}  // namespace widget
