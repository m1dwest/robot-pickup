#include "app.h"

#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Log.h>

int main() {
    plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);

    auto app = app::App{};

    auto ok = app.init_window(1280, 720, "Robot Control");
    if (!ok) {
        LOG_ERROR << "Couldn't initialize GUI application";
        return EXIT_FAILURE;
    }

    while (!app.should_close()) {
        app.update();
        app.compose();
        app.render();
        app.input();
    }
}
