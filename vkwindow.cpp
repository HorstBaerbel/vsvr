#include "vkwindow.h"

#include "vkutils.h"
#include <vector>
#include <iostream>

namespace vsvr
{

static constexpr uint32_t MAX_IN_FLIGHT_SUBMISSIONS = 2;

void Window::framebufferSizeCallback(glfw::Window* window, uint32_t width, uint32_t height)
{
    auto vkw = window->getUserPointer<Window>();
    vkw->framebufferResized();
}

Window::Window(uint32_t width, uint32_t height, const std::string &name)
    : m_size(width, height)
    , m_name(name)
{
}

Window::~Window()
{
}

void Window::framebufferResized()
{
    m_framebufferResized = true;
}

void Window::run()
{
    initWindow();
    initVulkan();
    init();
    mainLoop();
    vkDeviceWaitIdle(m_logicalDevice);
    cleanup();
    cleanupVulkan();
    cleanupWindow();
}

void Window::initWindow()
{
    glfw::init();
    glfw::Window::hint(GLFW_CLIENT_API, GLFW_NO_API);
    glfw::Window::hint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfw::createWindow(m_size, m_name.c_str());
    m_window.setUserPointer(this);
    m_window.setFramebufferSizeCallback(&framebufferSizeCallback);
}

void Window::cleanupWindow()
{
    m_window.destroy();
    glfw::terminate();
}

void Window::initVulkan()
{
    initInstance();
    initSurface();
    initDevices();
    initSwapChain();
    initRenderPass();
    initDescriptorPool();
    initPipeline();
    initFramebuffers();
    initCommandPool();
    initVertexBuffers();
    initCommandBuffers();
    initSyncObjects();
}

void Window::cleanupVulkan()
{
    //cleanupSyncObjects();
    cleanupSwapChain();
    cleanupVertexBuffers();
    cleanupCommandBuffers();
    cleanupCommandPool();
    cleanupFramebuffers();
    cleanupPipeline();
    cleanupDescriptorPool();
    cleanupRenderPass();
    cleanupDevices();
    cleanupSurface();
    cleanupInstance();
}

void Window::initInstance()
{
    // create application info struct
    vk::ApplicationInfo appInfo = {};
    appInfo.pApplicationName = m_name.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 7, 0);
    appInfo.pEngineName = "None";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 3, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;
    // create instance creation struct
    vk::InstanceCreateInfo createInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.pApplicationInfo = &appInfo;
    auto glfwExtensions = glfw::getRequiredInstanceExtensions<const char *>();
    createInfo.enabledExtensionCount = glfwExtensions.size();
    createInfo.ppEnabledExtensionNames = glfwExtensions.data();
#ifdef VULKAN_VALIDATE
    createInfo = m_validation.create(createInfo);
#endif
    m_instance = vk::createInstance(createInfo);
#ifdef VULKAN_VALIDATE
    m_validation.setup(m_instance);
#endif
}

void Window::cleanupInstance()
{
#ifdef VULKAN_VALIDATE
    m_validation.destroy();
#endif
    m_instance.destroy();
}

void Window::initSurface()
{
    m_surface = m_window.createSurface(m_instance);
}

void Window::cleanupSurface()
{
    m_instance.destroySurfaceKHR(m_surface);
}

void Window::initDevices()
{
    m_physicalDevice = pickPhysicalDevice(m_instance, m_surface);
    dumpDeviceInfo(m_physicalDevice);
    m_logicalDevice = createLogicalDevice(m_physicalDevice, m_surface);
    auto familyIndices = findQueueFamilies(m_physicalDevice, m_surface);
    m_graphicsQueue = m_logicalDevice.getQueue(familyIndices.graphicsFamily(), 0);
    m_presentQueue = m_logicalDevice.getQueue(familyIndices.presentFamily(), 0);
}

void Window::cleanupDevices()
{
    vkDestroyDevice(m_logicalDevice, nullptr);
}

void Window::initSwapChain()
{
    m_swapChain = createSwapChain(m_physicalDevice, m_logicalDevice, m_surface, m_size);
}

void Window::reinitSwapChain()
{
    // get new size of window
    auto size = m_window.getFramebufferSize();
    while (size.width == 0 || size.height == 0)
    {
        size = m_window.getFramebufferSize();
        glfw::waitEvents();
    }
    // wait for the device to finish rendering
    vkDeviceWaitIdle(m_logicalDevice);
    // destroy old swap chain
    cleanupSwapChain();
    // create new swap chain with new extent
    m_size = size;
    initSwapChain();
    initRenderPass();
    initDescriptorSetLayout();
    initPipeline();
    initFramebuffers();
    initCommandPool();
    initCommandBuffers();
    initSyncObjects();
}

void Window::cleanupSwapChain()
{
    cleanupSyncObjects();
    for (size_t i = 0; i < m_swapChain.framebuffers.size(); i++)
    {
        m_logicalDevice.destroyFramebuffer(m_swapChain.framebuffers[i]);
    }
    m_logicalDevice.freeCommandBuffers(m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    m_logicalDevice.destroyPipeline(m_graphicsPipeline);
    m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);
    m_logicalDevice.destroyRenderPass(m_renderPass);
    for (size_t i = 0; i < m_swapChain.imageViews.size(); i++)
    {
        m_logicalDevice.destroyImageView(m_swapChain.imageViews[i]);
    }
    m_logicalDevice.destroySwapchainKHR(m_swapChain.chain);
    cleanupDescriptorPool();
}

void Window::initRenderPass()
{
    vk::AttachmentDescription colorAttachment;
    colorAttachment.format = m_swapChain.surfaceFormat.format;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    vk::SubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    m_renderPass = m_logicalDevice.createRenderPass(renderPassInfo);
}

void Window::cleanupRenderPass()
{
}

void Window::initFramebuffers()
{
    m_swapChain = createSwapChainFramebuffers(m_logicalDevice, m_renderPass, m_swapChain);
}

void Window::cleanupFramebuffers()
{
}

void Window::initCommandPool()
{
    auto familyIndices = findQueueFamilies(m_physicalDevice, m_surface);
    m_commandPool = createCommandPool(m_logicalDevice, familyIndices.graphicsFamily());
    m_commandBuffers = allocateCommandBuffers(m_logicalDevice, m_commandPool, m_swapChain.framebuffers.size());
}

void Window::cleanupCommandPool()
{
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
}

void Window::initSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreInfo;
    m_presentComplete = m_logicalDevice.createSemaphore(semaphoreInfo);
    m_renderComplete = m_logicalDevice.createSemaphore(semaphoreInfo);
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    m_waitFences.resize(m_commandBuffers.size(), nullptr);
    for (auto &fence : m_waitFences)
    {
        fence = m_logicalDevice.createFence(fenceInfo);
    }
}

void Window::cleanupSyncObjects()
{
    m_logicalDevice.destroySemaphore(m_presentComplete);
    m_logicalDevice.destroySemaphore(m_renderComplete);
    for (auto &fence : m_waitFences)
    {
        m_logicalDevice.destroyFence(fence);
    }
}

void Window::mainLoop()
{
    while (!m_window.shouldClose())
    {
        glfw::pollEvents();
		// get next image in the swap chain (back/front buffer)
        uint32_t nextImageIndex = 0;
        auto result = m_logicalDevice.acquireNextImageKHR(m_swapChain.chain, UINT64_MAX, m_presentComplete, nullptr, &nextImageIndex);
        // check if we need to re-create the swap chain because it's out of date
        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            reinitSwapChain();
            return;
        }
        else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) 
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }
		// use a fence to wait until the command buffer has finished execution before using it again
		m_logicalDevice.waitForFences(1, &m_waitFences[nextImageIndex], VK_TRUE, UINT64_MAX);
		m_logicalDevice.resetFences(1, &m_waitFences[nextImageIndex]);
        // now call custom drawing function
        drawFrame();
        vk::PipelineStageFlags waitStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        vk::SubmitInfo submitInfo;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_presentComplete;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_renderComplete;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[nextImageIndex];
        // send rendering commands to queue
        m_graphicsQueue.submit(1, &submitInfo, m_waitFences[nextImageIndex]);
        // present image on window
        vk::PresentInfoKHR presentInfo;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderComplete;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapChain.chain;
        presentInfo.pImageIndices = &nextImageIndex;
        result = m_presentQueue.presentKHR(presentInfo);
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            reinitSwapChain();
        }
        else if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to present swap chain image!");
        }
        m_currentSubmission = (m_currentSubmission + 1) % MAX_IN_FLIGHT_SUBMISSIONS;
    }
}

} // namespace vsvr