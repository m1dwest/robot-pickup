#pragma once

#include <imgui.h>

namespace widget {

struct ImGuiChildScope {
    ImGuiChildScope(const char* str_id, const ImVec2& size = ImVec2(0, 0),
                    ImGuiChildFlags child_flags = 0,
                    ImGuiWindowFlags window_flags = 0) {
        ImGui::BeginChild(str_id, size, child_flags, window_flags);
    }

    ~ImGuiChildScope() { ImGui::EndChild(); }

    ImGuiChildScope(const ImGuiChildScope&) = delete;
    ImGuiChildScope& operator=(const ImGuiChildScope&) = delete;
};

struct ImGuiWindowScope {
    ImGuiWindowScope(const char* name, bool* p_open = NULL,
                     ImGuiWindowFlags flags = 0) {
        ImGui::Begin(name, p_open, flags);
    }

    ~ImGuiWindowScope() { ImGui::End(); }

    ImGuiWindowScope(const ImGuiChildScope&) = delete;
    ImGuiWindowScope& operator=(const ImGuiWindowScope&) = delete;
};

struct ImGuiGroupScope {
    ImGuiGroupScope() { ImGui::BeginGroup(); }
    ~ImGuiGroupScope() { ImGui::EndGroup(); }

    ImGuiGroupScope(const ImGuiGroupScope&) = delete;
    ImGuiGroupScope& operator=(const ImGuiGroupScope&) = delete;
};

}  // namespace widget
