cmake_minimum_required(VERSION 3.20)
project(select_menu)

add_library(select_menu STATIC
        select_menu.cpp
        select_menu.h
        select_menu_layer.cpp
        select_menu_layer.h
)

target_include_directories(select_menu PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(select_menu PUBLIC
        shader_engine_interface
)

