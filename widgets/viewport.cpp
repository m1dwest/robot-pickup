#include "viewport.h"

#include <glad/glad.h>
#include <plog/Log.h>

#include "guards.h"

namespace {

GLuint create_texture(int width, int height) {
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (texture == 0) {
        LOG_ERROR << "Failed to generate OpenGL texture";
    }

    return texture;
}

}  // namespace

namespace widget {

void Viewport::set_frame(const cv::Mat& frame) {
    if (_tex == 0) {
        _frame_w = frame.cols;
        _frame_h = frame.rows;
        _tex = create_texture(_frame_w, _frame_h);
    }

    glBindTexture(GL_TEXTURE_2D, _tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,     // mip level
                    0, 0,  // xoffset, yoffset
                    _frame_w, _frame_h,
                    GL_BGR,  // format of incoming data
                    GL_UNSIGNED_BYTE, frame.data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Viewport::set_scale(float scale) { _scale = scale; }

void Viewport::compose() {
    const auto size =
        ImVec2{static_cast<float>(_frame_w), static_cast<float>(_frame_h)};
    compose(size);
}

void Viewport::compose(const ImVec2& size) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const auto _ =
        widget::ImGuiChildScope("##frame_bg", size, ImGuiChildFlags_Borders,
                                ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    const auto region_size = ImGui::GetContentRegionAvail();

    const auto scale = [&region_size, this] {
        if (_scale == 0.0f) {
            const auto scale_x = region_size.x / _frame_w;
            const auto scale_y = region_size.y / _frame_h;
            return std::min(scale_x, scale_y);
        } else {
            return _scale;
        }
    }();

    const auto scaled_frame_w = _frame_w * scale;
    const auto scaled_frame_h = _frame_h * scale;
    const auto scaled_frame_size = ImVec2{scaled_frame_w, scaled_frame_h};

    const auto cursor_x =
        std::max((region_size.x - scaled_frame_w) / 2.0f, 0.0f);
    const auto cursor_y =
        std::max((region_size.y - scaled_frame_h) / 2.0f, 0.0f);
    const auto cursor_pos = ImVec2{cursor_x, cursor_y};

    const auto texture_id = (ImTextureID)(intptr_t)_tex;
    ImGui::SetCursorPos(cursor_pos);
    ImGui::Image(texture_id, scaled_frame_size);
}

}  // namespace widget
