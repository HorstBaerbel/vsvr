#pragma once

#include "vkincludes.h"
#include <vector>

namespace vsvr
{

/// @brief Info about queue families and their indices.
class QueueFamilyIndices
{
public:
    uint32_t graphicsFamily() const;
    void setGraphicsFamily(uint32_t index);
    uint32_t presentFamily() const;
    void setPresentFamily(uint32_t index);

    /// @brief Returns true if all families have been set.
    bool isComplete() const;

private:
    bool m_graphicsFamilySet = false;
    uint32_t m_graphicsFamily = 0;
    bool m_presentFamilySet = false;
    uint32_t m_presentFamily = 0;
};

/// @brief Find queue families available for physical device and surface.
QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

/// @brief Find index of device memory.
uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

/// @brief Pick the first physical device that supports Vulkan and has graphics capabilities.
/// @throw Throws if there are no GPUs supporting Vulkan.
vk::PhysicalDevice pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

/// @brief A logical device that supports Vulkan.
/// @throw Throws if there are no GPUs supporting Vulkan.
vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

/// @brief Dump information about the Vulkan device to stdout.
void dumpDeviceInfo(vk::PhysicalDevice physicalDevice);

/// @brief Swap chain information.
struct SwapChain
{
    vk::SwapchainKHR chain = nullptr;
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent = {0,0};
    std::vector<vk::Framebuffer> framebuffers;
};

/// @brief Create a swap chain for device.
SwapChain createSwapChain(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::SurfaceKHR surface, vk::Extent2D extent, vk::Format format = vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear);

/// @brief Create frame buffers for a swap chain.
SwapChain createSwapChainFramebuffers(vk::Device logicalDevice, vk::RenderPass renderPass, const SwapChain &swapChain);

/// @brief Create command pool for queue family.
vk::CommandPool createCommandPool(vk::Device logicalDevice, uint32_t familyIndex, vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlags());

/// @brief Allocate command buffers in command pool.
std::vector<vk::CommandBuffer> allocateCommandBuffers(vk::Device logicalDevice, vk::CommandPool commandPool, uint32_t count);

}
