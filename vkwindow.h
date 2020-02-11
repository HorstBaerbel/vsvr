#pragma once

#include "vkdevice.h"
#include "vkincludes.h"
#include <stdexcept>
#include <cstdlib>
#include <memory>
#include <string>

namespace vsvr
{

class Window
{
public:
    /// @brief Create new Window.
    Window(uint32_t width = 480, uint32_t height = 320, const std::string &name = "");
    /// @brief Destroy window and clean up.
    virtual ~Window();

    /// @brief Called when the size of the GLFW window changes.
    virtual void framebufferResized();

    void run();

protected:
    /// @brief Initialize graphics. Called after window and have been set up.
    virtual void init() = 0;
    /// @brief Custom draw function. Use command buffers to draw. Afterwards this will 
    /// vkQueueSubmit the command buffers to the graphics vk::Queue and call 
    /// vkQueuePresentKHR on the present vk::Queue to display the framebuffers.
    virtual void drawFrame() = 0;
    /// @brief De-initialize graphics. Called before and window are destroyed.
    virtual void cleanup() = 0;

    glfw::Window m_window;
    std::string m_name;
    vk::Extent2D m_size = {0,0};
    bool m_framebufferResized = false;

#ifdef VULKAN_VALIDATE
    Validation m_validation;
#endif
    vk::Instance m_instance = nullptr;
    vk::SurfaceKHR m_surface = nullptr;
    vk::PhysicalDevice m_physicalDevice = nullptr;
    vk::Device m_logicalDevice = nullptr;
    vk::Queue m_graphicsQueue = nullptr;
    vk::Queue m_presentQueue = nullptr;
    SwapChain m_swapChain;
    vk::RenderPass m_renderPass = nullptr;
    vk::PipelineLayout m_pipelineLayout = nullptr;
    vk::Pipeline m_graphicsPipeline = nullptr;
    vk::CommandPool m_commandPool = nullptr;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    vk::Semaphore m_presentComplete;
	vk::Semaphore m_renderComplete;
    std::vector<vk::Fence> m_waitFences;
    size_t m_currentSubmission = 0;

    virtual void initInstance();
    virtual void cleanupInstance();
    virtual void initSurface();
    virtual void cleanupSurface();
    virtual void initDevices();
    virtual void cleanupDevices();
    virtual void initSwapChain();
    virtual void reinitSwapChain();
    virtual void cleanupSwapChain();
    virtual void initRenderPass();
    virtual void cleanupRenderPass();
    virtual void initDescriptorPool() = 0;
    virtual void cleanupDescriptorPool() = 0;
    virtual void initDescriptorSetLayout() = 0;
    virtual void cleanupDescriptorSetLayout() = 0;
    virtual void initDescriptorSets() = 0;
    virtual void cleanupDescriptorSets() = 0;
    virtual void initPipeline() = 0;
    virtual void cleanupPipeline() = 0;
    virtual void initFramebuffers();
    virtual void cleanupFramebuffers();
    virtual void initCommandPool();
    virtual void cleanupCommandPool();
    virtual void initVertexBuffers() = 0;
    virtual void cleanupVertexBuffers() = 0;
    virtual void initCommandBuffers() = 0;
    virtual void cleanupCommandBuffers() = 0;
    virtual void initSyncObjects();
    virtual void cleanupSyncObjects();

private:
    static void framebufferSizeCallback(glfw::Window* window, uint32_t width, uint32_t height);

    void initWindow();
    void cleanupWindow();
    void initVulkan();
    void cleanupVulkan();
    void mainLoop();    
};

}