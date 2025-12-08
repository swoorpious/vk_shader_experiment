cmake_minimum_required(VERSION 3.20)
project(vk_shader_engine)

set(CMAKE_CXX_STANDARD 20)


# --------------------------------------
# Vulkan
option(BUILD_LOADER "Build Vulkan loader" ON)
option(BUILD_TESTS "Build tests" OFF)

add_subdirectory(external/Vulkan-Headers)
add_subdirectory(external/Vulkan-Loader)


# --------------------------------------
# glslang
set(ENABLE_GLSLANG_BINARIES ON CACHE BOOL "" FORCE)
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glslang)

file(GLOB SHADER_SOURCES
        ${CMAKE_SOURCE_DIR}/shaders/*.vert
        ${CMAKE_SOURCE_DIR}/shaders/*.frag
)

set(COMPILED_SHADERS "")

foreach (SHADER ${SHADER_SOURCES})
    get_filename_component(NAME ${SHADER} NAME)
    set(OUT ${CMAKE_BINARY_DIR}/shaders/${NAME}.spv)

    add_custom_command(
            OUTPUT ${OUT}
            COMMAND glslangValidator -V ${SHADER} -o ${OUT}
            DEPENDS ${SHADER}
            COMMENT "Compiling shader ${NAME}"
    )

    list(APPEND COMPILED_SHADERS ${OUT})
endforeach ()

add_custom_target(Shaders ALL DEPENDS ${COMPILED_SHADERS})



# --------------------------------------
# sdl3
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(external/sdl)


add_subdirectory(shader_repo)
#add_subdirectory(external)


# --------------------------------------
# dear_imgui
add_library(imgui
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_demo.cpp

        # Backends
        external/imgui/backends/imgui_impl_sdl3.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
)


target_link_libraries(imgui
        PUBLIC
        SDL3::SDL3
        Vulkan::Headers
        Vulkan::Loader
)

# --------------------------------------
# project
add_executable(vk_shader_engine 
        src/main.cpp
        src/engine.cpp
        include/engine.h
        include/util/viewport.h
        src/util/viewport.cpp
        src/util/math.cpp
        include/util/math.h
        src/core/engine_object.cpp
        include/core/engine_object.h
        src/core/layer_component.cpp
        include/core/layer_component.h
)
target_link_libraries(vk_shader_engine
        PRIVATE
        Vulkan::Headers
        Vulkan::Loader
        SDL3::SDL3
        imgui
        vk_shader_repo
)

target_include_directories(vk_shader_engine
        PRIVATE
        external/Vulkan-Headers/include
        external/Vulkan-Loader/loader
        external/sdl/include
        include/
        shader_repo/
)




