#pragma once

#include <memory>
#include <optional>
#include <string>

#include "state.h"
#include "views/view.h"
#include "vision/camera.h"

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

    State _state;

    std::unique_ptr<View> _view;
    vision::Camera _camera;
};

}  // namespace app
