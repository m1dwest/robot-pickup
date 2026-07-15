cmake_minimum_required(VERSION 3.16)

project(robot-pickup CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(plog CONFIG REQUIRED)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER APP_SOURCES EXCLUDE REGEX "/build/")

add_executable(bin ${APP_SOURCES})
set_target_properties(bin PROPERTIES CXX_EXTENSIONS ON)

target_link_libraries(bin PRIVATE  imgui::imgui glfw glad::glad plog::plog)
