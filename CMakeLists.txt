cmake_minimum_required(VERSION 3.16)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")
set(CMAKE_C_STANDARD 11)


add_subdirectory(SDL EXCLUDE_FROM_ALL)

project(station_to_station LANGUAGES C CXX)

add_executable(station_to_station)

target_sources(station_to_station PRIVATE 
    src/main.c

    imgui/imgui.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlgpu3.cpp
)

target_compile_options(station_to_station PRIVATE -O3)
target_compile_options(station_to_station PRIVATE -fpermissive)

target_include_directories(station_to_station PRIVATE inc ${CMAKE_SOURCE_DIR})
target_include_directories(station_to_station PRIVATE inc ${CMAKE_SOURCE_DIR}/SDL)
target_include_directories(station_to_station PRIVATE inc ${CMAKE_SOURCE_DIR}/imgui)
target_include_directories(station_to_station PRIVATE inc ${CMAKE_SOURCE_DIR}/imgui/backends)

target_link_libraries(station_to_station PRIVATE
    m
    SDL3::SDL3
)