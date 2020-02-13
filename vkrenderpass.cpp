#include "vkrenderpass.h"

#include "vkutils.h"
#include <stdexcept>

namespace vsvr
{

DEVICERESOURCE_FUNCTIONS_CPP(RenderPass)

RenderPass & RenderPass::operator=(RenderPass &&other)
{
    if (&other != this)
	{
		DeviceResource::operator=(std::move(other));
        m_pass = std::move(other.m_pass); other.m_pass = nullptr;
        m_currentCommandBuffer = std::move(other.m_currentCommandBuffer); other.m_currentCommandBuffer = nullptr;
	}
	return *this;
}

void RenderPass::create(vk::Device logicalDevice, const Settings &settings)
{
     if (isValid())
    {
        throw std::runtime_error("PenderPass already created!");
    }
    // copy settings as we need to possibly modify them
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.setAttachmentCount(static_cast<uint32_t>(settings.colorAttachments.size()));
    renderPassInfo.setPAttachments(settings.colorAttachments.data());
    renderPassInfo.setSubpassCount(static_cast<uint32_t>(settings.subpasses.size()));
    renderPassInfo.setPSubpasses(settings.subpasses.data());
    renderPassInfo.setDependencyCount(static_cast<uint32_t>(settings.dependencies.size()));
    renderPassInfo.setPDependencies(settings.dependencies.data());
    m_pass = logicalDevice.createRenderPass(renderPassInfo);
    setCreated(logicalDevice);
}

void RenderPass::destroyResource()
{
    vkDestroyRenderPass(logicalDevice(), m_pass, nullptr);
    m_pass = nullptr;
}

const vk::RenderPass RenderPass::pass() const
{
    return m_pass;
}

void RenderPass::begin(vk::CommandBuffer commandBuffer, const vk::RenderPassBeginInfo &passBeginInfo, vk::SubpassContents subPassContents)
{
    if (m_currentCommandBuffer)
    {
        throw std::runtime_error("Render pass currently bound!");
    }
    commandBuffer.beginRenderPass(passBeginInfo, subPassContents);
    m_currentCommandBuffer = commandBuffer;
}

void RenderPass::end()
{
    if (!m_currentCommandBuffer)
    {
        throw std::runtime_error("Render pass not currently bound!");
    }
    m_currentCommandBuffer.endRenderPass();
    m_currentCommandBuffer = nullptr;
}

} // namespace vsvr
