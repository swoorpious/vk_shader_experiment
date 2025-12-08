// copyright 2025 swaroop.

#include <util/viewport.h>
#include <SDL3/SDL.h>
#include <cstdio>

void Viewport::init(SDL_Window* windowHandle) {
    window = windowHandle;
    updateFromWindow();
}

void Viewport::onResize() {
    if (!window) return;
    
    printf("[viewport]: windows has been resized\n");
    updateFromWindow();
}

void Viewport::updateFromWindow() {
    int w, h;
    int pixelW, pixelH;

    SDL_GetWindowSize(window, &w, &h);

    printf("[viewport] x: %f, y: %f\n", w, h);
    
    logicalSize = { static_cast<float>(w), static_cast<float>(h) };
    
    SDL_GetWindowSizeInPixels(window, &pixelW, &pixelH);
    pixelSize = { static_cast<float>(pixelW), static_cast<float>(pixelH) };

    float scale = SDL_GetWindowDisplayScale(window);
    contentScale = { scale, scale };

    size = pixelSize;

    const Uint32 flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN) {
        state = Fullscreen;
    } else if (flags & SDL_WINDOW_BORDERLESS) {
        state = WindowedFullscreen;
    } else {
        state = Windowed;
    }
}

VkViewport Viewport::toVkViewport(bool flipY) const {
    VkViewport vp{};
    vp.x = position.x;
    
    if (flipY) {
        vp.y = position.y + size.y;
        vp.height = -size.y;
    } else {
        vp.y = position.y;
        vp.height = size.y;
    }

    vp.width = size.x;
    vp.minDepth = depthRange.x;
    vp.maxDepth = depthRange.y;

    return vp;
}

VkRect2D Viewport::toScissor() const {
    VkRect2D scissor{};
    scissor.offset = { static_cast<int32_t>(position.x), static_cast<int32_t>(position.y) };
    scissor.extent = { static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y) };
    return scissor;
}

Math::Vector2f Viewport::getSize() const { return size; }
Math::Vector2f Viewport::getMinSize() const { return minSize; }
Math::Vector2f Viewport::getPosition() const { return position; }
Math::Vector2f Viewport::getDepthRange() const { return depthRange; }
Math::Vector2f Viewport::getPixelSize() const { return pixelSize; }
Math::Vector2f Viewport::getLogicalSize() const { return logicalSize; }
Math::Vector2f Viewport::getContentScale() const { return contentScale; }
float Viewport::getAspectRatio() const { 
    return (size.y > 0.0f) ? (size.x / size.y) : 1.0f; 
}