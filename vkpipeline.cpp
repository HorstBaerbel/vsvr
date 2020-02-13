#include "vkpipeline.h"

#include "vkutils.h"
#include <algorithm>

namespace vsvr
{

DEVICERESOURCE_FUNCTIONS_CPP(PipelineLayout)

PipelineLayout &PipelineLayout::operator=(PipelineLayout &&other)
{
    if (&other != this)
    {
        DeviceResource::operator=(std::move(other));
        m_layout = std::move(other.m_layout); other.m_layout = nullptr;
    }
    return *this;
}

void PipelineLayout::create(vk::Device logicalDevice, const Settings & settings)
{
    if (isValid())
    {
        throw std::runtime_error("PipelineLayout already created!");
    }
    std::vector<vk::DescriptorSetLayout> dsls;
    std::transform(settings.descriptorSetLayouts.cbegin(),settings.descriptorSetLayouts.cend(), std::back_inserter(dsls), [](DescriptorSetLayout::ConstPtr dsl){ return dsl->layout(); });
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = dsls.size();
    pipelineLayoutInfo.pSetLayouts = dsls.data();
    pipelineLayoutInfo.pushConstantRangeCount = settings.pushConstants.size();
    pipelineLayoutInfo.pPushConstantRanges = settings.pushConstants.data();
    m_layout = logicalDevice.createPipelineLayout(pipelineLayoutInfo);
    setCreated(logicalDevice);
}

const vk::PipelineLayout PipelineLayout::layout() const
{
    return m_layout;
}

void PipelineLayout::destroyResource()
{
    vkDestroyPipelineLayout(logicalDevice(), m_layout, nullptr);
    m_layout = nullptr;
}

//-------------------------------------------------------------------------------------------------

Pipeline::Settings Pipeline::Settings::Default()
{
    Settings settings;
    // draw triangle lists
    settings.inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    // framebuffer max. z = 1.0
    settings.viewport.maxDepth = 1.0f;
    // draw filled polygons, cull back faces (that is counter-clockwise winding),
    // because Vulkan has right-hand coordinates in NDC!
    settings.rasterization.polygonMode = vk::PolygonMode::eFill;
    settings.rasterization.lineWidth = 1.0f;
    settings.rasterization.cullMode = vk::CullModeFlagBits::eBack;
    settings.rasterization.frontFace = vk::FrontFace::eClockwise;
    // no multisampling
    settings.multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    // no blending, simple copying
    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;
    // don't yet set up attachmentCount and pAttachments here! Do that on pipeline creation
    settings.colorBlendAttachments.push_back(colorBlendAttachment);
    settings.colorBlending.logicOp = vk::LogicOp::eCopy;
    return settings;
}

DEVICERESOURCE_FUNCTIONS_CPP(Pipeline)

Pipeline &Pipeline::operator=(Pipeline &&other)
{
    if (&other != this)
    {
        DeviceResource::operator=(std::move(other));
        m_pipeline = std::move(other.m_pipeline); other.m_pipeline = nullptr;
    }
    return *this;
}

void Pipeline::create(vk::Device logicalDevice, RenderPass::ConstPtr renderPass, const PipelineLayout &layout, const Settings &settings)
{
    // copy settings as we need to possibly modify them
    Settings s = settings;
    // dynamic states
    vk::PipelineDynamicStateCreateInfo dynamicState;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(s.dynamicStates.size());
    dynamicState.pDynamicStates = s.dynamicStates.data();
    // vertex input / bindings
    vk::PipelineVertexInputStateCreateInfo vertexInput;
    vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(s.vertexBindings.size());
    vertexInput.pVertexBindingDescriptions = s.vertexBindings.data();
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(s.attributeBindings.size());
    vertexInput.pVertexAttributeDescriptions = s.attributeBindings.data();
    // viewport / scissors
    vk::PipelineViewportStateCreateInfo viewportState;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &s.viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &s.scissors;
    // blending
    s.colorBlending.attachmentCount = s.colorBlendAttachments.size();
    s.colorBlending.pAttachments = s.colorBlendAttachments.data();
    // shaders
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    for (const auto & shader : s.shaderStages)
    {
        vk::PipelineShaderStageCreateInfo stageInfo;
        stageInfo.stage = shader->stage();
        stageInfo.module = shader->module();
        stageInfo.pName = shader->entryPoint().data();
        shaderStages.push_back(stageInfo);
    }
    // create pipeline
    vk::GraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &s.inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &s.rasterization;
    pipelineInfo.pMultisampleState = &s.multisampling;
    pipelineInfo.pColorBlendState = &s.colorBlending;
    pipelineInfo.layout = layout.layout();
    pipelineInfo.renderPass = renderPass->pass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;
    pipelineInfo.basePipelineIndex = -1;
    m_pipeline = logicalDevice.createGraphicsPipeline(nullptr, pipelineInfo);
    setCreated(logicalDevice);
}

void Pipeline::destroyResource()
{
    logicalDevice().destroyPipeline(m_pipeline);
    m_pipeline = nullptr;
}

const vk::Pipeline Pipeline::pipeline() const
{
    return m_pipeline;
}

void Pipeline::bind(vk::CommandBuffer commandBuffer, vk::PipelineBindPoint bindPoin)
{
}

} // namespace vsvr
