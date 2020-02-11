#include "vkshader.h"

#include "vkutils.h"
#include <stdexcept>
#include <fstream>

namespace vsvr
{

RESOURCE_FUNCTIONS_CPP(Shader)

Shader & Shader::operator=(Shader &&other)
{
    if (&other != this)
	{
		Resource::operator=(std::move(other));
        m_module = std::move(other.m_module); other.m_module = nullptr;
        m_stage = std::move(other.m_stage);
        m_entryPoint = std::move(other.m_entryPoint);
	}
	return *this;
}

void Shader::create(vk::Device logicalDevice, const std::vector<char> &code, vk::ShaderStageFlagBits stage, const std::string &entryPoint)
{
    if (isValid())
    {
        throw std::runtime_error("Shader already created!");
    }
    m_module = createShader(logicalDevice, code);
    m_stage = stage;
    m_entryPoint = entryPoint;
    setCreated(logicalDevice);
}

void Shader::create(vk::Device logicalDevice, const std::string &fileName, vk::ShaderStageFlagBits stage, const std::string &entryPoint)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open shader file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    create(logicalDevice, buffer, stage, entryPoint);
}

void Shader::destroyResource()
{
    logicalDevice().destroyShaderModule(m_module);
    m_module = nullptr;
}

const vk::ShaderModule Shader::module() const
{
    return m_module;
}

vk::ShaderStageFlagBits Shader::stage() const
{
    return m_stage;
}

const std::string &Shader::entryPoint() const
{
    return m_entryPoint;
}

vk::ShaderModule Shader::createShader(vk::Device logicalDevice, const std::vector<char> &code)
{
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    vk::ShaderModule shaderModule;
    shaderModule = logicalDevice.createShaderModule(createInfo);
    return shaderModule;
}

} // namespace vsvr
