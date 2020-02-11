#include "vkutils.h"

namespace vsvr
{

std::map<vk::PhysicalDevice, vk::PhysicalDeviceProperties> DeviceInfoCache::m_propertiesCache;
std::map<vk::PhysicalDevice, vk::PhysicalDeviceMemoryProperties> DeviceInfoCache::m_memoryPropertiesCache;

const vk::PhysicalDeviceProperties & DeviceInfoCache::getProperties(vk::PhysicalDevice physicalDevice)
{
    auto pcIt = m_propertiesCache.find(physicalDevice);
    if (pcIt != m_propertiesCache.cend())
    {
        return pcIt->second;
    }
    m_propertiesCache[physicalDevice] = physicalDevice.getProperties();
    return m_propertiesCache[physicalDevice];
}

const vk::PhysicalDeviceMemoryProperties & DeviceInfoCache::getMemoryProperties(vk::PhysicalDevice physicalDevice)
{
    auto pcIt = m_memoryPropertiesCache.find(physicalDevice);
    if (pcIt != m_memoryPropertiesCache.cend())
    {
        return pcIt->second;
    }
    m_memoryPropertiesCache[physicalDevice] = physicalDevice.getMemoryProperties();
    return m_memoryPropertiesCache[physicalDevice];
}

}