#pragma once

#include "vkresource.h"
#include "vkdescriptor.h"
#include "vkshader.h"
#include "vkrenderpass.h"
#include "vkincludes.h"
#include <vector>
#include <string>
#include <memory>

namespace vsvr
{

class PipelineLayout: public Resource
{
public:
    /// @brief Encapsulates all relevant pipline layout settings.
    struct Settings
    {
        std::vector<DescriptorSetLayout::ConstPtr> descriptorSetLayouts;
        std::vector<vk::PushConstantRange> pushConstants;
    };

    RESOURCE_FUNCTIONS_H(PipelineLayout)

    /// @brief Layout constructor. Will allocate buffer and device memory.
    void create(vk::Device logicalDevice, const Settings & settings = Settings());

    /// @brief Get piepline layout.
    const vk::PipelineLayout layout() const;

private:
    vk::PipelineLayout m_layout = nullptr;
};

class Pipeline: public Resource
{
public:
    struct Settings
    {
        std::vector<vk::DynamicState> dynamicStates;
        std::vector<vk::VertexInputBindingDescription> vertexBindings;
        std::vector<vk::VertexInputAttributeDescription> attributeBindings;
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
        vk::Viewport viewport = {};
        vk::Rect2D scissors = {};
        vk::PipelineRasterizationStateCreateInfo rasterization = {};
        vk::PipelineMultisampleStateCreateInfo multisampling = {};
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments = {};
        vk::PipelineColorBlendStateCreateInfo colorBlending = {};
        std::vector<Shader::ConstPtr> shaderStages;

        /// @brief Pre-initalized pipeline settings.
        static Settings Default();
    };

    RESOURCE_FUNCTIONS_H(Pipeline)

    /// @brief Create pipeline.
    void create(vk::Device logicalDevice, RenderPass::ConstPtr renderPass, const PipelineLayout &layout, const Settings &settings);

    /// @brief Get pipeline handle.
    const vk::Pipeline pipeline() const;

    /// @brief Bind the pipeline for a specific command buffer.
    void bind(vk::CommandBuffer commandBuffer, vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics);

private:
    vk::Pipeline m_pipeline = nullptr;
};

}
