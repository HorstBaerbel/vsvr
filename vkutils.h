#pragma once

#include "vkincludes.h"
#include <stdexcept>
#include <type_traits>
#include <map>

namespace vsvr
{

class DeviceInfoCache
{
public:
    static const vk::PhysicalDeviceProperties &getProperties(vk::PhysicalDevice physicalDevice);
    static const vk::PhysicalDeviceMemoryProperties &getMemoryProperties(vk::PhysicalDevice physicalDevice);

private:
    static std::map<vk::PhysicalDevice, vk::PhysicalDeviceProperties> m_propertiesCache;
    static std::map<vk::PhysicalDevice, vk::PhysicalDeviceMemoryProperties> m_memoryPropertiesCache;
};

/// @brief Check Vulkan return value of f and throw std::runtime_error with string s if != VK_SUCCESS.
#define VK_CHECK_THROW(f, s){if ((f) != VK_SUCCESS) { throw std::runtime_error(s); }}

}