#include "app.h"

#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>
#include <plog/Log.h>
#include <librealsense2/rs.hpp>

#include "views/main_view.h"
#include "vision/aruco.h"

namespace {

struct InitError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

static void glfw_error_callback(int error, const char* description) {
    LOG_ERROR << "GLFW error: " << error << ", " << description;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
    auto app = static_cast<app::App*>(glfwGetWindowUserPointer(window));
    if (app) {
        // app->handle_key(key, action);
    } else {
        LOG_ERROR << "Unable to process keyboard input. No window user pointer "
                     "attached";
    }
}

static void framebuffer_size_callback(GLFWwindow* window, int width,
                                      int height) {
    glViewport(0, 0, width, height);
}

void init_glfw() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        const char* log = nullptr;
        glfwGetError(&log);

        auto what = "Couldn't initialize GLFW" +
                    ((log != nullptr) ? ": " + std::string(log) : "");
        throw InitError{std::move(what)};
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    std::string glsl_version = "";
#ifdef __APPLE__
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // required on Mac OS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#elif __linux__
    glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#elif _WIN32
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _WIN32
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    LOG_INFO << "Monitor scale: " << xscale << "x" << yscale;
    if (xscale > 1 || yscale > 1) {
        highDPIscaleFactor = xscale;
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    }
#elif __APPLE__
    // to prevent 1200x800 from becoming 2400x1600
    // and some other weird resizings
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
}

void init_glad() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw InitError{"Couldn't initialize GLAD"};
    }

    LOG_INFO << "OpenGL renderer: " << glGetString(GL_RENDERER);
    LOG_INFO << "OpenGL from glad " << GLVersion.major << "."
             << GLVersion.minor;
}

void init_imgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        throw InitError{"Couldn't initialize Dear ImGui GLFW implementation"};
    }

    if (!ImGui_ImplOpenGL3_Init()) {
        throw InitError{"Couldn't initialize Dear ImGui OpenGL implementation"};
    }
}

app::App::Window create_window(int width, int height, std::string&& title,
                               bool is_vsync_enabled) {
    app::App::Window window;
    // const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    // glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    window.width = width;
    window.height = height;
    window.title = std::move(title);
    window.window = glfwCreateWindow(window.width,   // mode->width,
                                     window.height,  // mode->height,
                                     window.title.c_str(),
                                     NULL,  // monitor
                                     NULL);
    if (!window.window) {
        throw InitError{"Couldn't create a GLFW window"};
    }

    glfwSetWindowPos(window.window, 100, 100);
    glfwSetWindowSizeLimits(window.window,
                            static_cast<int>(width * window.hiDPIScale),
                            static_cast<int>(height * window.hiDPIScale),
                            GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetFramebufferSizeCallback(window.window, framebuffer_size_callback);
    glfwMakeContextCurrent(window.window);
    glfwSwapInterval(static_cast<int>(is_vsync_enabled));

    LOG_INFO << "OpenGL from GLFW "
             << glfwGetWindowAttrib(window.window, GLFW_CONTEXT_VERSION_MAJOR)
             << "."
             << glfwGetWindowAttrib(window.window, GLFW_CONTEXT_VERSION_MINOR);

    return window;
}

}  // namespace

namespace app {

App::App() : _camera(1280, 720, 30) { _view = std::make_unique<MainView>(); }

App::~App() {
    //
}

bool App::should_close() const {
    return glfwWindowShouldClose(_window->window);
}

void App::update() {
    _state.camera_frame = _camera.wait_for_frame();

    vision::Aruco aruco;
    aruco.detect(_state.camera_frame);
    _state.aruco_detections = aruco.detections();

    _view->update(_state);
}

void App::compose() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    _view->compose();

    ImGui::End();
}

void App::render() {
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(_window->window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(_window->window);
}

void App::input() {
    glfwPollEvents();

    // for (const auto& cmd : _view_manager.pop_current_commands()) {
    //     execute(cmd, [this, &cmd](std::string_view msg) {
    //         if (auto dest = command::route_for(cmd)) {
    //             show_error(_view_manager, *dest, msg);
    //         } else {
    //             LOG_ERROR << "Error not shown in GUI: " << msg;
    //         }
    //     });
    // }
}

bool App::init_window(unsigned width, unsigned height, std::string title) {
    try {
        init_glfw();
        LOG_INFO << "GLFW initialized";

        _window =
            create_window(width, height, std::move(title), _is_vsync_enabled);
        glfwSetWindowUserPointer(_window->window, this);
        glfwSetKeyCallback(_window->window, key_callback);
        LOG_INFO << "GLFW window created";

        init_glad();
        LOG_INFO << "GLAD initialized";

        init_imgui(_window->window);
        LOG_INFO << "Dear ImGui initialized";
    } catch (InitError e) {
        LOG_ERROR << "Initialization failed:";
        LOG_ERROR << e.what();
    }

    return true;
}

}  // namespace app
