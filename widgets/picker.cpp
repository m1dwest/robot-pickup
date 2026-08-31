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
    const auto x = std::format(
        "X: {}",
        _clicked_pos.has_value()
            ? std::to_string(static_cast<int>(std::roundf(_clicked_pos->x)))
            : "");
    ImGui::TextUnformatted(x.c_str());

    const auto y = std::format(
        "Y: {}",
        _clicked_pos.has_value()
            ? std::to_string(static_cast<int>(std::roundf(_clicked_pos->y)))
            : "");
    ImGui::TextUnformatted(y.c_str());

    const auto depth = std::format(
        "Depth: {}",
        _depth.has_value() ? std::format("{:.2f}", _depth.value()) : "");
    ImGui::TextUnformatted(depth.c_str());
    ImGui::EndDisabled();
}

bool Picker::is_enabled() const { return _is_enabled; }

void Picker::set_clicked_pos(const std::optional<ImVec2>& pos) {
    _clicked_pos = pos;
}

void Picker::set_depth(const std::optional<float>& depth) { _depth = depth; }

}  // namespace widget
