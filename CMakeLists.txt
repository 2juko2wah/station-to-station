cmake_minimum_required(VERSION 3.16)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

project(station_to_station)

add_compile_options(-O3)
add_executable(station_to_station)

target_sources(station_to_station PRIVATE 
    src/main.c
    src/vec2.c
    src/agent.c
) 
target_compile_features(station_to_station PUBLIC c_std_99)
target_include_directories(station_to_station PRIVATE inc)

add_subdirectory(SDL EXCLUDE_FROM_ALL)

target_link_libraries(station_to_station PUBLIC
    m
    SDL3::SDL3
)