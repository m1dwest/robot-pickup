#include "app.h"

#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Log.h>

int main() {
    plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);

    auto app = app::App{};

    try {
        app.init_window(1280, 720, "Robot Control");
        app.init_camera(640, 480, 30);
    } catch (const app::InitError& e) {
        LOG_ERROR << "Couldn't initialize GUI application:";
        LOG_ERROR << e.what();
        return EXIT_FAILURE;
    } catch (const rs2::error& e) {
        LOG_ERROR << "Couldn't initialize RealSense camera:";
        LOG_ERROR << e.what();
        LOG_ERROR << "Failed function: " << e.get_failed_function();
        return EXIT_FAILURE;
    }

    while (!app.should_close()) {
        app.update();
        app.compose();
        app.render();
        app.input();
    }
}
