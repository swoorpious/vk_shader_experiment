cmake_minimum_required(VERSION 4.0)
project(vk_shader_exp)

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


# --------------------------------------
# project
add_executable(vk_shader_exp 
        src/main.cpp
        src/Engine.cpp
        src/Engine.h
)
target_link_libraries(vk_shader_exp
        PRIVATE
        Vulkan::Headers
        Vulkan::Loader
        SDL3::SDL3
)

target_include_directories(vk_shader_exp
        PRIVATE
        external/Vulkan-Headers/include
        external/Vulkan-Loader/loader
        external/sdl/include
        
)
