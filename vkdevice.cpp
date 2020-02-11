#include "vkdevice.h"

#include "vkutils.h"
#include <set>
#include <stdexcept>
#include <iostream>

namespace vsvr
{

uint32_t QueueFamilyIndices::graphicsFamily() const
{
    return m_graphicsFamily;
}

void QueueFamilyIndices::setGraphicsFamily(uint32_t index)
{
    m_graphicsFamily = index;
    m_graphicsFamilySet = true;
}

uint32_t QueueFamilyIndices::presentFamily() const
{
    return m_presentFamily;
}

void QueueFamilyIndices::setPresentFamily(uint32_t index)
{
    m_presentFamily = index;
    m_presentFamilySet = true;
}

bool QueueFamilyIndices::isComplete() const
{
    return m_graphicsFamilySet && m_presentFamilySet;
}

QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); familyIndex++)
    {
        const auto &queueFamily = queueFamilies.at(familyIndex);
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.setGraphicsFamily(familyIndex);
        }
        auto presentSupport = physicalDevice.getSurfaceSupportKHR(familyIndex, surface);
        if (presentSupport)
        {
            indices.setPresentFamily(familyIndex);
        }
        if (indices.isComplete())
        {
            break;
        }
    }
    return indices;
}

uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t typeBits, vk::MemoryPropertyFlags properties)
{
    auto memProperties = DeviceInfoCache::getMemoryProperties(physicalDevice);
    // iterate over all memory types available for the device
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeBits & 1) == 1)
        {
            if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }
        typeBits >>= 1;
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

// ------------------------------------------------------------------------------------------------

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool checkDeviceExtensionSupport(vk::PhysicalDevice physicalDevice)
{
    auto deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto &property : deviceExtensionProperties)
    {
        requiredExtensions.erase(property.extensionName);
    }
    return requiredExtensions.empty();
}

// ------------------------------------------------------------------------------------------------

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats, vk::Format format, vk::ColorSpaceKHR colorSpace)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == format && availableFormat.colorSpace == colorSpace)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D extent)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        vk::Extent2D actualExtent = extent;
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    SwapChainSupportDetails details;
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
    return details;
}

SwapChain createSwapChain(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::SurfaceKHR surface, vk::Extent2D extent, vk::Format format, vk::ColorSpaceKHR colorSpace)
{
    // check swap chain capabilities, surface format and present mode
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, format, colorSpace);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities, extent);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    // create swap chain
    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    // check if our graphics family is equal to our present family
    // in that case there's no need to share images between the two queues
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily(), indices.presentFamily()};
    if (indices.graphicsFamily() != indices.presentFamily())
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;
    SwapChain swapChain;
    swapChain.chain = logicalDevice.createSwapchainKHR(createInfo);
    // get buffers / images from swap chain
    swapChain.images = logicalDevice.getSwapchainImagesKHR(swapChain.chain);
    swapChain.surfaceFormat = surfaceFormat;
    swapChain.extent = swapChainExtent;
    // create image views for swap chain images
    swapChain.imageViews.resize(swapChain.images.size());
    for (size_t i = 0; i < swapChain.images.size(); i++)
    {
        vk::ImageViewCreateInfo createInfo;
        createInfo.image = swapChain.images[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = swapChain.surfaceFormat.format;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        swapChain.imageViews[i] = logicalDevice.createImageView(createInfo);
    }
    return swapChain;
}

SwapChain createSwapChainFramebuffers(vk::Device logicalDevice, vk::RenderPass renderPass, const SwapChain &swapChain)
{
    SwapChain result = swapChain;
    result.framebuffers.resize(swapChain.imageViews.size());
    for (size_t i = 0; i < swapChain.imageViews.size(); i++)
    {
        vk::ImageView attachments[] = {swapChain.imageViews[i]};
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChain.extent.width;
        framebufferInfo.height = swapChain.extent.height;
        framebufferInfo.layers = 1;
        result.framebuffers[i] = logicalDevice.createFramebuffer(framebufferInfo);
    }
    return result;
}

// ------------------------------------------------------------------------------------------------

bool isDeviceSuitable(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

vk::PhysicalDevice pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
{
    auto physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.empty())
    {
        throw std::runtime_error("Failed to find any GPUs with Vulkan support!");
    }
    for (const auto &device : physicalDevices)
    {
        // find a device that supports graphics operations
        if (isDeviceSuitable(device, surface))
        {
            return device;
        }
    }
    throw std::runtime_error("Failed to find a suitable GPU!");
}

vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    auto indices = findQueueFamilies(physicalDevice, surface);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily(), indices.presentFamily()};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::DeviceCreateInfo createInfo;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // note that for Vulkan < 1.1 we would need to set up validation layers here too for devices!
    createInfo.enabledLayerCount = 0;
    return physicalDevice.createDevice(createInfo);
}

void dumpDeviceInfo(vk::PhysicalDevice physicalDevice)
{
    auto devProps = DeviceInfoCache::getProperties(physicalDevice);
    auto memProps = DeviceInfoCache::getMemoryProperties(physicalDevice);
    uint64_t vramSize = 0;
    for (int i = 0; i < memProps.memoryHeapCount; i++)
    {
        if (memProps.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal)
        {
            vramSize = memProps.memoryHeaps[i].size;
            break;
        }
    }
    std::cout << "GPU: " << devProps.deviceName << ", VRAM: " << vramSize << ", driver: " << devProps.driverVersion << std::endl;
    std::cout << "Vulkan version: " << devProps.apiVersion << std::endl;
}

vk::CommandPool createCommandPool(vk::Device logicalDevice, uint32_t familyIndex, vk::CommandPoolCreateFlags flags)
{
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = familyIndex;
    poolInfo.flags = flags;// Optional, e.g. VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    vk::CommandPool commandPool;
    return logicalDevice.createCommandPool(poolInfo);
}

std::vector<vk::CommandBuffer> allocateCommandBuffers(vk::Device logicalDevice, vk::CommandPool commandPool, uint32_t count)
{
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = count;
    return logicalDevice.allocateCommandBuffers(allocInfo);
}

} // namespace vsvr