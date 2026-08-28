#include "picker.h"

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

    if (_is_enabled) {
        const auto x_label =
            std::format("X: {}", _clicked_pos.has_value()
                                     ? std::format("{:.2f}", _clicked_pos->x)
                                     : "");
        ImGui::TextUnformatted(x_label.c_str());

        const auto y_label =
            std::format("Y: {}", _clicked_pos.has_value()
                                     ? std::format("{:.2f}", _clicked_pos->y)
                                     : "");
        ImGui::TextUnformatted(y_label.c_str());
    } else {
        ImGui::TextDisabled("X:");
        ImGui::TextDisabled("Y:");
    }
}

bool Picker::is_enabled() const { return _is_enabled; }

void Picker::set_clicked_pos(const std::optional<ImVec2>& pos) {
    _clicked_pos = pos;
}

}  // namespace widget
