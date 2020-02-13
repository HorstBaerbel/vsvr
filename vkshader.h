#pragma once

#include "vkresource.h"
#include "vkincludes.h"
#include <string>
#include <vector>
#include <memory>

namespace vsvr
{

class Shader: public DeviceResource
{
public:
    DEVICERESOURCE_FUNCTIONS_H(Shader)

    /// @brief Construct a shader module from SPIR-V code.
    void create(vk::Device logicalDevice, const std::vector<char> &code, vk::ShaderStageFlagBits stage, const std::string &entryPoint = "main");
    /// @brief Load SPIR-V shader code from a file and construct shader module.
    void create(vk::Device logicalDevice, const std::string &fileName, vk::ShaderStageFlagBits stage, const std::string &entryPoint = "main");

    /// @brief Get the shaders module.
    const vk::ShaderModule module() const;
    /// @brief Get the shader stage.
    vk::ShaderStageFlagBits stage() const;
    /// @brief Get the shaders entry point.
    const std::string &entryPoint() const;

private:
    static vk::ShaderModule createShader(vk::Device logicalDevice, const std::vector<char> &code);

    vk::ShaderModule m_module = nullptr;
    vk::ShaderStageFlagBits m_stage;
    std::string m_entryPoint;
};

}
