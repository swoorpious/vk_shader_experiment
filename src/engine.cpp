// copyright 2025 swaroop.

#include <engine.h>
#include <core/engine_object.h>
#include <select_menu/select_menu.h>

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl3.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <algorithm>

Engine::Engine() {
    initSDL();
    initWindow();
    initVulkan();
    createImGuiPool();
    createImGuiRenderPass();
    
    createSwapchain();
    createFramebuffers();
    createCommandBuffers();
    
    initImGui();
}

Engine::~Engine() {
    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    for (auto fb : framebuffers)
        vkDestroyFramebuffer(device, fb, nullptr);
    framebuffers.clear();

    vkFreeCommandBuffers(device, commandPool,
                         static_cast<uint32_t>(commandBuffers.size()),
                         commandBuffers.data());

    if (swapchain) vkDestroySwapchainKHR(device, swapchain, nullptr);
    if (imguiRenderPass) vkDestroyRenderPass(device, imguiRenderPass, nullptr);
    if (imguiPool) vkDestroyDescriptorPool(device, imguiPool, nullptr);
    if (device) vkDestroyDevice(device, nullptr);
    if (surface) vkDestroySurfaceKHR(instance, surface, nullptr);
    if (instance) vkDestroyInstance(instance, nullptr);
    if (window) SDL_DestroyWindow(window);

    SDL_Quit();
    printf("shutdown complete\n");
}

void Engine::switchProject(EngineObject* new_app) {
    if (current_app) delete current_app;
    current_app = new_app;
    if (current_app) current_app->onSetup();
}

void Engine::run() {
    // load the first EngineObject subclass to begin
    switchProject(new SelectMenuObject(this));

    uint64_t lastTime = SDL_GetPerformanceCounter();

    bool running = true;
    while (running) {
        const uint64_t now = SDL_GetPerformanceCounter();
        const float deltaTime = static_cast<float>(now - lastTime) / static_cast<float>(SDL_GetPerformanceFrequency());
        lastTime = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            
            // handle window resize
            if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                if (event.window.windowID == SDL_GetWindowID(window)) {
                    int w = 0, h = 0;
                    SDL_GetWindowSize(window, &w, &h);
                    
                    // only resize if the window has valid dimensions - not minimized
                    if (w > 0 && h > 0) {
                        viewport.onResize();
                        vkDeviceWaitIdle(device);

                        for (auto fb : framebuffers) 
                            vkDestroyFramebuffer(device, fb, nullptr);
                        framebuffers.clear();

                        vkFreeCommandBuffers(device, commandPool, 
                                           static_cast<uint32_t>(commandBuffers.size()), 
                                           commandBuffers.data());
                        commandBuffers.clear();

                        if (swapchain) {
                            vkDestroySwapchainKHR(device, swapchain, nullptr);
                            swapchain = VK_NULL_HANDLE;
                        }

                        createSwapchain();
                        createFramebuffers();
                        createCommandBuffers();
                    }
                }
            }
        }

        if (swapchainExtent.width == 0 || swapchainExtent.height == 0) {
            SDL_Delay(100); 
            continue;
        }

        // setup/render imgui
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // tick EngineObject on every iteration
        if (current_app) {
            current_app->update(deltaTime);
            
            /*
             * needs to be called before ImGui::Render()
             * cannot render before calling ImGui::NewFrame() and rendering all the layers inside EngineObject
             */
            current_app->render(); // calls internal render hook (which does nothing lol)
        }
        ImGui::Render();

        // vulkan bullshit
        VkCommandBuffer cmd = commandBuffers[currentFrame];
        
        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
        VkRenderPassBeginInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.renderPass = imguiRenderPass;
        rpInfo.framebuffer = framebuffers[currentFrame];
        rpInfo.renderArea.extent = swapchainExtent;
        rpInfo.clearValueCount = 1;
        rpInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        /*
         * this line here renders imgui
         */
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cmd;
        
        vkQueueSubmit(graphicsQueue, 1, &si, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        VkPresentInfoKHR pi{};
        pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        pi.swapchainCount = 1;
        pi.pSwapchains = &swapchain;


        uint32_t imageIndex = static_cast<uint32_t>(currentFrame);
        pi.pImageIndices = &imageIndex;

        VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &pi);
        
        // handle swapchain invalidation during present - some drivers might signal it here
        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
            // usually handled within the event loop
            // currently we rely on the SDL event to trigger the recreation logic above
        }

        currentFrame = (currentFrame + 1) % framebuffers.size();
    }
}


void Engine::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForVulkan(window);

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = instance;
    info.PhysicalDevice = physicalDevice;
    info.Device = device;
    info.QueueFamily = graphicsQueueFamily; // required by the version of imgui in use
    info.Queue = graphicsQueue;
    info.DescriptorPool = imguiPool;
    info.MinImageCount = 2;
    info.ImageCount = 2;
    
    /*
     * imgui now uses ImGui_ImplVulkan_PipelineInfo instead of RenderPassData
     *
     * following would exist outside the ImGui_ImplVulkan_PipelineInfo struct
     * info.MSAASamples
     * info.Renderpass
     */
    info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.PipelineInfoMain.RenderPass = imguiRenderPass;
    info.PipelineInfoMain.Subpass = 0;

    ImGui_ImplVulkan_Init(&info);

}

void Engine::initSDL() {
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error("SDL init failed");
}

void Engine::initWindow() {
    Math::Vector2f size = viewport.getSize();
    Math::Vector2f min_size = viewport.getMinSize();
    window = SDL_CreateWindow("engine", size.x, size.y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) throw std::runtime_error("window creation failed");

    SDL_SetWindowMinimumSize(window, min_size.x, min_size.y);
    viewport.init(window);
}

void Engine::initVulkan() {
    uint32_t extCount = 0;
    const char* const* extNames = SDL_Vulkan_GetInstanceExtensions(&extCount);

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.enabledExtensionCount = extCount;
    ci.ppEnabledExtensionNames = extNames;

    if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("instance creation failed");

    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
        throw std::runtime_error("surface creation failed");

    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (gpuCount == 0) throw std::runtime_error("no GPUs found");

    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());
    physicalDevice = gpus[0];

    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qp(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &qCount, qp.data());

    for (uint32_t i = 0; i < qCount; i++) {
        if (qp[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueFamily = i;
            break;
        }
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = graphicsQueueFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &priority;

    const char* deviceExtensions[] = { "VK_KHR_swapchain" };
    VkDeviceCreateInfo dci{};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(physicalDevice, &dci, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("device creation failed");

    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

    VkCommandPoolCreateInfo cpi{};
    cpi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpi.queueFamilyIndex = graphicsQueueFamily;
    cpi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(device, &cpi, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("command pool creation failed");
}

void Engine::createImGuiPool() {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }, // font texture
    };
    VkDescriptorPoolCreateInfo pi{};
    pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pi.maxSets = 1;
    pi.poolSizeCount = 1;
    pi.pPoolSizes = poolSizes;
    if (vkCreateDescriptorPool(device, &pi, nullptr, &imguiPool) != VK_SUCCESS)
        throw std::runtime_error("imgui pool creation failed");
}

void Engine::createImGuiRenderPass() {
    VkAttachmentDescription color{};
    color.format = VK_FORMAT_B8G8R8A8_UNORM;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref{};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub{};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rp{};
    rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp.attachmentCount = 1;
    rp.pAttachments = &color;
    rp.subpassCount = 1;
    rp.pSubpasses = &sub;
    rp.dependencyCount = 1;
    rp.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &rp, nullptr, &imguiRenderPass) != VK_SUCCESS)
        throw std::runtime_error("render pass creation failed");
}

void Engine::createSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);
    swapchainExtent = caps.currentExtent;

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    VkSurfaceFormatKHR chosenFormat = formats[0];
    for (auto &f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosenFormat = f;
            break;
        }
    }

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface;
    sci.minImageCount = std::clamp(caps.minImageCount + 1, caps.minImageCount, caps.maxImageCount > 0 ? caps.maxImageCount : 100);
    sci.imageFormat = chosenFormat.format;
    sci.imageColorSpace = chosenFormat.colorSpace;
    sci.imageExtent = swapchainExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &sci, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swapchain");
}

void Engine::createFramebuffers() {
    uint32_t imgCount;
    vkGetSwapchainImagesKHR(device, swapchain, &imgCount, nullptr);
    std::vector<VkImage> swapchainImages(imgCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imgCount, swapchainImages.data());

    framebuffers.resize(imgCount);

    for (size_t i = 0; i < imgCount; i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
        viewInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        VkImageView imageView;
        vkCreateImageView(device, &viewInfo, nullptr, &imageView);

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = imguiRenderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &imageView;
        fbInfo.width = swapchainExtent.width;
        fbInfo.height = swapchainExtent.height;
        fbInfo.layers = 1;

        vkCreateFramebuffer(device, &fbInfo, nullptr, &framebuffers[i]);
    }
}

void Engine::createCommandBuffers() {
    commandBuffers.resize(framebuffers.size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
}

VkCommandBuffer Engine::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &beginInfo);
    return cmd;
}

void Engine::endSingleTimeCommands(VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);
    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    vkQueueSubmit(graphicsQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &cmd);
}