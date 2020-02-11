#include "vkvalidation.h"

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

namespace vsvr
{

// ----- debug report / LunarG --------------------------------------------------------------------

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportPrint(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
    std::cerr << "Validation layer: " << pMessage << std::endl;
    return VK_FALSE;
}

vk::DebugReportCallbackCreateInfoEXT GenerateDebugReportCallbackCreateInfo()
{
    vk::DebugReportCallbackCreateInfoEXT info;
    info.pNext = nullptr;
    info.flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning;
    info.pfnCallback = &DebugReportPrint;
    info.pUserData = nullptr;
    return info;
}

// ----- debug utils / Khronos --------------------------------------------------------------------

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsPrint(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

vk::DebugUtilsMessengerCreateInfoEXT GenerateDebugUtilsMessengerCreateInfo()
{
    vk::DebugUtilsMessengerCreateInfoEXT info;
    info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    info.pfnUserCallback = &DebugUtilsPrint;
    return info;
}

// ----- common -----------------------------------------------------------------------------------

const std::map<Validation::LayerType, Validation::LayerInfo> Validation::SupportedValidationLayers = {
    {Validation::LayerType::KHRONOS, {{"VK_LAYER_KHRONOS_validation"}, {VK_EXT_DEBUG_UTILS_EXTENSION_NAME}}},
    {Validation::LayerType::LUNARG, {{"VK_LAYER_LUNARG_standard_validation"}, {VK_EXT_DEBUG_REPORT_EXTENSION_NAME}}}};

Validation::LayerType Validation::getSupportedLayerType()
{
    auto availableLayerProperties = vk::enumerateInstanceLayerProperties();
    for (auto layerEntry : SupportedValidationLayers)
    {
        auto layerInfo = layerEntry.second;
        for (auto layer : layerInfo.layers)
        {
            for (const auto &layerProperties : availableLayerProperties)
            {
                if (strcmp(layer, layerProperties.layerName) == 0)
                {
                    return layerEntry.first;
                }
            }
        }
    }
    return LayerType::NONE;
}

vk::InstanceCreateInfo Validation::create(const vk::InstanceCreateInfo &srcInfo)
{
    vk::InstanceCreateInfo createInfo = srcInfo;
    m_validationLayerType = getSupportedLayerType();
    if (m_validationLayerType != LayerType::NONE)
    {
        m_validationLayer = SupportedValidationLayers.at(m_validationLayerType);
        m_requiredExtensions = glfw::getRequiredInstanceExtensions<const char *>();
        std::copy(m_validationLayer.extensions.cbegin(), m_validationLayer.extensions.cend(), std::back_inserter(m_requiredExtensions));
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = m_requiredExtensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayer.layers.size());
        createInfo.ppEnabledLayerNames = m_validationLayer.layers.data();
        if (m_validationLayerType == LayerType::KHRONOS)
        {
            m_debugUtilsCreateInfo = GenerateDebugUtilsMessengerCreateInfo();
            createInfo.pNext = (vk::DebugUtilsMessengerCreateInfoEXT *)&m_debugUtilsCreateInfo;
        }
        else if (m_validationLayerType == LayerType::LUNARG)
        {
            m_debugReportCreateInfo = GenerateDebugReportCallbackCreateInfo();
            createInfo.pNext = (vk::DebugReportCallbackCreateInfoEXT *)&m_debugReportCreateInfo;
        }
    }
    return createInfo;
}

void Validation::setup(vk::Instance instance)
{
    if (!instance)
    {
        throw std::runtime_error("Null instance passed!");
    }
    m_instance = instance;
    if (m_validationLayerType == LayerType::KHRONOS)
    {
        m_debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(GenerateDebugUtilsMessengerCreateInfo());
        std::cerr << "Validation layer " << m_validationLayer.layers.front() << " enabled." << std::endl;
    }
    else if (m_validationLayerType == LayerType::LUNARG)
    {
        m_debugReportCallback = instance.createDebugReportCallbackEXT(GenerateDebugReportCallbackCreateInfo());
        std::cerr << "Validation layer " << m_validationLayer.layers.front() << " enabled." << std::endl;
    }
}

void Validation::destroy()
{
    if (!m_instance)
    {
        return;
    }
    if (m_validationLayerType == LayerType::KHRONOS)
    {
        if (m_debugUtilsMessenger)
        {
            m_instance.destroyDebugUtilsMessengerEXT(m_debugUtilsMessenger);
        }
    }
    else if (m_validationLayerType == LayerType::LUNARG)
    {
        if (m_debugReportCallback)
        {
            m_instance.destroyDebugReportCallbackEXT(m_debugReportCallback);
        }
    }
}

} // namespace vsvr