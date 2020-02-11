#pragma once

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <type_traits>

namespace glfw
{

/// @copydoc glfwInit()
inline void init()
{
    glfwInit();
}

/// @copydoc glfwWaitEvents()
inline void waitEvents()
{
    glfwWaitEvents();
}

/// @copydoc glfwPollEvents()
inline void pollEvents()
{
    glfwPollEvents();
}

/// @copydoc glfwTerminate()
inline void terminate()
{
    glfwTerminate();
}

class Window
{
public:
    using FramebufferSizeCallback = void (*)(Window*,uint32_t,uint32_t);

    Window()
    {
    }

    Window(GLFWwindow * window)
        : m_window(window)
    {
        glfwSetWindowUserPointer(m_window, this);
    }

    /// @copydoc glfwSetWindowUserPointer(GLFWwindow* window, void* pointer)
    void setUserPointer(void *pointer)
    {
        m_userPointer = pointer;
    }

    /// @copydoc glfwSetWindowUserPointer(GLFWwindow* window)
    template <typename T>
    T * getUserPointer()
    {
        return reinterpret_cast<T *>(m_userPointer);
    }

    /// @copydoc glfwSetFramebufferSizeCallback(GLFWwindow* window, GLFWframebuffersizefun cbfun)
    FramebufferSizeCallback setFramebufferSizeCallback(FramebufferSizeCallback cbfun)
    {
        m_frameBufferSizeCallback = cbfun;
        glfwSetFramebufferSizeCallback(m_window, frameBufferSizeCallback);
        return cbfun;
    }

    FramebufferSizeCallback getFramebufferSizeCallback() const
    {
        return m_frameBufferSizeCallback;
    }

    /// @copydoc glfwGetFramebufferSize(GLFWwindow* window, int *width, int *height)
    vk::Extent2D getFramebufferSize()
    {
        int width = 0; int height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        return vk::Extent2D(width, height);
    }

    /// @copydoc glfwCreateWindowSurface(VkInstance instance, GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
    vk::SurfaceKHR createSurface(vk::Instance instance, vk::AllocationCallbacks *allocator = nullptr)
    {
        VkAllocationCallbacks vka = allocator != nullptr ? (*allocator) : vk::AllocationCallbacks();
        VkSurfaceKHR surface = nullptr;
        if (glfwCreateWindowSurface(instance, m_window, allocator != nullptr ? &vka : nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("glfw::createWindowSurface failed!");
        }
        return vk::SurfaceKHR(surface);
    }

    /// @copydoc glfwWindowShouldClose(GLFWwindow* window)
    bool shouldClose()
    {
        return glfwWindowShouldClose(m_window);
    }

    /// @copydoc glfwDestroyWindow(GLFWwindow* window)
    inline void destroy()
    {
        if (m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    /// @copydoc glfwWindowHint(int hint, int value)
    static void hint(int hint, int value)
    {
        glfwWindowHint(hint, value);
    }

private:
    static void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        auto w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        w->getFramebufferSizeCallback()(w, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    GLFWwindow *m_window = nullptr;
    void *m_userPointer = nullptr;
    FramebufferSizeCallback m_frameBufferSizeCallback = nullptr;
};

/// @copydoc glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
inline Window createWindow(vk::Extent2D size, const char *title, GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr)
{
    return Window(glfwCreateWindow(static_cast<int>(size.width), static_cast<int>(size.height), title, monitor, share));
}

/// @brief Get required Vulkan extensions for GLFW.
template <typename T>
inline std::vector<T> getRequiredInstanceExtensions();

/// @brief Get required Vulkan extensions for GLFW.
template <>
inline std::vector<std::string> getRequiredInstanceExtensions()
{
    std::vector<std::string> result;
    uint32_t count = 0;
    auto extensions = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t i = 0; i < count; i++)
    {
        result.push_back(std::string(extensions[i]));
    }
    return result;
}

/// @brief Get required Vulkan extensions for GLFW.
template <>
inline std::vector<const char *> getRequiredInstanceExtensions()
{
    std::vector<const char *> result;
    uint32_t count = 0;
    auto extensions = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t i = 0; i < count; i++)
    {
        result.push_back(extensions[i]);
    }
    return result;
}

} // namespace glfw
