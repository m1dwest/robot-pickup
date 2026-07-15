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
        _w = frame.cols;
        _h = frame.rows;
        _tex = create_texture(_w, _h);
    }

    glBindTexture(GL_TEXTURE_2D, _tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,     // mip level
                    0, 0,  // xoffset, yoffset
                    _w, _h,
                    GL_BGR,  // format of incoming data
                    GL_UNSIGNED_BYTE, frame.data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Viewport::set_scale(float scale) { _scale = scale; }

void Viewport::compose() {
    const auto viewport_scale = ImGui::GetContentRegionAvail().x / _w;
    const auto viewport_size = ImVec2(_w * viewport_scale, _h * viewport_scale);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    const auto _ = widget::ImGuiChildScope("##frame_bg", viewport_size,
                                           ImGuiChildFlags_None);
    ImGui::PopStyleColor();
    const auto region_size = ImGui::GetContentRegionAvail();
    const auto cursor_pos = ImGui::GetCursorPos();
    const auto geometry = calc_frame_geometry(region_size, cursor_pos);

    const auto texture_id = (ImTextureID)(intptr_t)_tex;
    ImGui::SetCursorPos(geometry.pos);
    ImGui::Image(texture_id, geometry.size, geometry.uv_0, geometry.uv_1);
}

Viewport::FrameGeometry Viewport::calc_frame_geometry(const ImVec2& available,
                                                      const ImVec2& frame_pos) {
    const auto frame_w = static_cast<float>(_w);
    const auto frame_h = static_cast<float>(_h);

    // const auto ui_scale = (scale == 0.0f)
    //                           ? 1.0f
    //                           : std::min(1.0f, available.x / (frame_w *
    //                           scale));
    const auto ui_frame_w = (_scale == 0.0f)
                                ? available.x
                                : std::min(available.x, available.x * _scale);
    const auto ui_scale = ui_frame_w / frame_w;
    const auto ui_frame_h = frame_h * ui_scale;

    const auto ui_frame_x = (available.x - ui_frame_w) / 2.0f;
    const auto ui_frame_y = (available.y - ui_frame_h) / 2.0f;

    const auto uv_scale = (_scale == 0.0f)
                              ? 1.0f
                              : std::min(1.0f, ui_frame_w / (frame_w * _scale));
    const auto uv_0 = (1.0f - uv_scale) / 2.0f;
    const auto uv_1 = uv_0 + uv_scale;

    return {.pos = {ui_frame_x, ui_frame_y},
            .size = {ui_frame_w, ui_frame_h},
            .uv_0 = {uv_0, uv_0},
            .uv_1 = {uv_1, uv_1}};
}

}  // namespace widget
