#pragma once

#include <optional>
#include <string>

struct GLFWwindow;

namespace app {

class App {
   public:
    struct Window {
        int width;
        int height;
        std::string title;
        GLFWwindow* window = nullptr;
        float hiDPIScale = 1.0f;
    };

    App();
    ~App();

    bool should_close() const;
    void update();
    void compose();
    void render();
    void input();

    [[nodiscard]] bool init_window(unsigned width, unsigned height,
                                   std::string title);

   private:
    bool _is_vsync_enabled = true;
    bool _is_fullscreen = false;

    std::optional<Window> _window;

    int _speed = 0;
    int _speed_max = 1000;
    int _speed_min = 0;

    const char* _vtype_items[2] = {"Control point", "Position angular"};
    int _vtype = 0;

    const char* _sframe_items[2] = {"Base coordinate", "Robot coordinate"};
    int _sframe = 0;

    int _x_axis = 0;
    int _y_axis = 0;
    int _z_axis = 0;
    int _axis_max = 100;
    int _axis_min = -100;

    float _rx = 0.0f;
    float _ry = 0.0f;
    float _rz = 0.0f;
    float _elbow = 0.0f;

    float _rot_max = 450000.0f;
    float _rot_min = -450000.0f;
};

}  // namespace app
