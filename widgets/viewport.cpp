#include "viewport.h"

#include <glad/glad.h>
#include <imgui.h>
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

void draw_polygons(
    const std::vector<widget::Viewport::OverlayPolygon>& polygons,
    ImDrawList* draw_list, ImVec2 cursor_screen_pos, float scale) {
    for (const auto& polygon : polygons) {
        std::vector<ImVec2> points;
        points.reserve(polygon.points.size());

        for (const auto& point : polygon.points) {
            points.emplace_back(cursor_screen_pos.x + point.x * scale,
                                cursor_screen_pos.y + point.y * scale);
        }

        draw_list->AddPolyline(
            points.data(), static_cast<int>(points.size()), polygon.color,
            polygon.closed ? ImDrawFlags_Closed : ImDrawFlags_None,
            polygon.thickness);
    }
}

void draw_labels(const std::vector<widget::Viewport::OverlayLabel>& labels,
                 ImDrawList* draw_list, ImVec2 cursor_screen_pos, float scale) {
    for (const auto& label : labels) {
        const auto pos = ImVec2{cursor_screen_pos.x + label.pos.x * scale,
                                cursor_screen_pos.y + label.pos.y * scale};

        draw_list->AddText(pos, label.color, label.text.c_str());
    }
}

// TODO: pass transformer function
void draw_points(const std::vector<widget::Viewport::OverlayPoint>& points,
                 ImDrawList* draw_list, ImVec2 cursor_screen_pos, float scale) {
    for (const auto& point : points) {
        const auto pos = ImVec2{cursor_screen_pos.x + point.pos.x * scale,
                                cursor_screen_pos.y + point.pos.y * scale};

        draw_list->AddCircleFilled(pos, point.radius, point.color);
    }
}

void draw_lines(const std::vector<widget::Viewport::OverlayLine>& lines,
                ImDrawList* draw_list, ImVec2 cursor_screen_pos, float scale) {
    for (const auto& line : lines) {
        const auto pos_1 = ImVec2{cursor_screen_pos.x + line.p_1.x * scale,
                                  cursor_screen_pos.y + line.p_1.y * scale};
        const auto pos_2 = ImVec2{cursor_screen_pos.x + line.p_2.x * scale,
                                  cursor_screen_pos.y + line.p_2.y * scale};
        draw_list->AddLine(pos_1, pos_2, line.color, line.thickness);
    }
}

widget::Viewport::OverlayPoint create_point_overlay(const ImVec2& point) {
    return widget::Viewport::OverlayPoint{
        .pos = {point.x, point.y},
        .radius = 3.0f,
        .color = IM_COL32(255, 0, 0, 255),
    };
}

}  // namespace

namespace widget {

void Viewport::set_frame(const cv::Mat& frame) {
    if (_frame_tex == 0) {
        _frame_w = frame.cols;
        _frame_h = frame.rows;
        _frame_tex = create_texture(_frame_w, _frame_h);
    }

    glBindTexture(GL_TEXTURE_2D, _frame_tex);
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

void Viewport::update_clicked_pos(ClickedButton clickedButton) {
    ImVec2 item_min = ImGui::GetItemRectMin();
    ImVec2 mouse = ImGui::GetMousePos();

    auto viewport_x = mouse.x - item_min.x;
    auto viewport_y = mouse.y - item_min.y;

    auto [clicked_pos, overlay] =
        clickedButton == ClickedButton::Left
            ? std::make_pair(&_clicked_pos_l, &_overlay_point_l)
            : std::make_pair(&_clicked_pos_r, &_overlay_point_r);
    *clicked_pos = {viewport_x / _true_scale, viewport_y / _true_scale};
    *overlay = create_point_overlay(clicked_pos->value());
}

std::optional<ImVec2> Viewport::get_clicked_pos_l() const {
    return _clicked_pos_l;
}

std::optional<ImVec2> Viewport::get_clicked_pos_r() const {
    return _clicked_pos_r;
}

void Viewport::clear_overlay() {
    _overlay_polygons.clear();
    _overlay_labels.clear();
    _overlay_points.clear();
    _overlay_lines.clear();
}

void Viewport::add_overlay_polygon(OverlayPolygon polygon) {
    _overlay_polygons.push_back(std::move(polygon));
}

void Viewport::add_overlay_label(OverlayLabel label) {
    _overlay_labels.push_back(std::move(label));
}

void Viewport::add_overlay_distance(OverlayPoint p_1, OverlayPoint p_2,
                                    float distance) {
    _overlay_points.push_back(p_1);
    _overlay_points.push_back(p_2);

    auto line = OverlayLine{.p_1 = p_1.pos,
                            .p_2 = p_2.pos,
                            .thickness = 2.0f,
                            .color = IM_COL32(255, 0, 0, 255)};
    _overlay_lines.push_back(std::move(line));

    const auto h_offset = 20.0f;
    auto label_pos = cv::Point2f{(p_1.pos.x + p_2.pos.x) / 2.0f + h_offset,
                                 (p_1.pos.y + p_2.pos.y) / 2.0f};
    auto label =
        OverlayLabel{.pos = std::move(label_pos),
                     .text = std::format("{:.2f} mm", distance * 1000.0f),
                     .color = IM_COL32(255, 0, 0, 255)};
    _overlay_labels.push_back(std::move(label));
}

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

    _true_scale = [&region_size, this] {
        if (_scale == 0.0f) {
            const auto scale_x = region_size.x / _frame_w;
            const auto scale_y = region_size.y / _frame_h;
            return std::min(scale_x, scale_y);
        } else {
            return _scale;
        }
    }();

    const auto scaled_frame_w = _frame_w * _true_scale;
    const auto scaled_frame_h = _frame_h * _true_scale;
    const auto scaled_frame_size = ImVec2{scaled_frame_w, scaled_frame_h};

    const auto cursor_x =
        std::max((region_size.x - scaled_frame_w) / 2.0f, 0.0f);
    const auto cursor_y =
        std::max((region_size.y - scaled_frame_h) / 2.0f, 0.0f);
    const auto cursor_pos = ImVec2{cursor_x, cursor_y};

    const auto texture_id = (ImTextureID)(intptr_t)_frame_tex;
    ImGui::SetCursorPos(cursor_pos);
    const auto image_screen_pos = ImGui::GetCursorScreenPos();

    ImGui::Image(texture_id, scaled_frame_size);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        update_clicked_pos(ClickedButton::Left);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        update_clicked_pos(ClickedButton::Right);
    }

    auto* draw_list = ImGui::GetWindowDrawList();

    draw_polygons(_overlay_polygons, draw_list, image_screen_pos, _true_scale);
    draw_labels(_overlay_labels, draw_list, image_screen_pos, _true_scale);
    draw_points(_overlay_points, draw_list, image_screen_pos, _true_scale);

    std::vector<OverlayPoint> clicked_points;
    if (_overlay_point_l.has_value()) {
        clicked_points.push_back(_overlay_point_l.value());
    }
    if (_overlay_point_r.has_value()) {
        clicked_points.push_back(_overlay_point_r.value());
    }
    draw_points(clicked_points, draw_list, image_screen_pos, _true_scale);
    draw_lines(_overlay_lines, draw_list, image_screen_pos, _true_scale);
}

}  // namespace widget
