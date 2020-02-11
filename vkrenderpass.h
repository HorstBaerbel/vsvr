#pragma once

#include "vkresource.h"
#include "vkincludes.h"
#include <memory>
#include <vector>

namespace vsvr
{

class RenderPass: public Resource
{
public:
    struct Settings
    {
        std::vector<vk::AttachmentDescription> colorAttachments = {};
        std::vector<vk::AttachmentReference> colorAttachmentRefs = {};
        std::vector<vk::SubpassDescription> subpasses = {};
        std::vector<vk::SubpassDependency> dependencies = {};
    };

    RESOURCE_FUNCTIONS_H(RenderPass)

    /// @brief Construct a render pass.
    void create(vk::Device logicalDevice, const Settings &settings);

    /// @brief Get the render pass.
    const vk::RenderPass pass() const;

    /// @brief Begin recording the render pass.
    void begin(vk::CommandBuffer commandBuffer, const vk::RenderPassBeginInfo &passBeginInfo, vk::SubpassContents subPassContents = vk::SubpassContents::eInline);
    /// @brief Stop recording the render pass.
    void end();

private:
    vk::RenderPass m_pass = nullptr;
    vk::CommandBuffer m_currentCommandBuffer = nullptr;
};

}
