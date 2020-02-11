#pragma once

#include "vkincludes.h"
#include <vector>
#include <map>

namespace vsvr
{

class Validation
{
public:
    /// @brief The type of validation layer supported / enabled.
    enum class LayerType { NONE, KHRONOS, LUNARG };
    
    /// @brief Create validation. Call with pre-filled vk::InstanceCreateInfo
    /// Will return updated vk::InstanceCreateInfo. Use to call vkCreateInstance().
    vk::InstanceCreateInfo create(const vk::InstanceCreateInfo &createInfo);

    /// @brief Set up validation. Call after instance created with vkCreateInstance().
    void setup(vk::Instance instance);

    /// @brief Clean up validation. Call before vkDestroyInstance().
    /// The destructor will NOT clean up.
    void destroy();

private:
    struct LayerInfo
    {
        std::vector<const char *> layers;
        std::vector<const char *> extensions;
    };

    static const std::map<LayerType, LayerInfo> SupportedValidationLayers;
    static LayerType getSupportedLayerType();

    LayerType m_validationLayerType = LayerType::NONE;
    LayerInfo m_validationLayer;
    std::vector<const char *> m_requiredExtensions;
    vk::Instance m_instance = nullptr;

    vk::DebugReportCallbackCreateInfoEXT m_debugReportCreateInfo;
    vk::DebugReportCallbackEXT m_debugReportCallback = nullptr;
    vk::DebugUtilsMessengerCreateInfoEXT m_debugUtilsCreateInfo;
    vk::DebugUtilsMessengerEXT m_debugUtilsMessenger = nullptr;
};

}
