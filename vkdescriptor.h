#pragma once

#include "vkbuffer.h"
#include "vkresource.h"
#include "vkincludes.h"
#include <vector>
#include <memory>

namespace vsvr
{

/// @brief
class DescriptorSetLayout: public Resource
{
public:
    RESOURCE_FUNCTIONS_H(DescriptorSetLayout)

    /// @brief Get descriptor set layout.
    const vk::DescriptorSetLayout layout() const;

    /// @brief Create descriptor set layout.
    void create(vk::Device logicalDevice, const std::vector<vk::DescriptorSetLayoutBinding> &bindings);

private:
    vk::DescriptorSetLayout m_layout = nullptr;
};

class DescriptorPool;

/// @brief
class DescriptorSet: public Resource
{
public:
    RESOURCE_FUNCTIONS_H(DescriptorSet)

    /// @brief Get descriptor set handle.
    const vk::DescriptorSet set() const;

    /// @brief Copy data to device memory.
    void update(const std::vector<void *> &data, const std::vector<size_t> &size);

    /// @brief Bind the buffer for a specific command buffer.
    void bind(vk::CommandBuffer commandBuffer, uint32_t firstBinding = 0);

protected:
    /// @brief Create descriptor set. Can not be called from outside.
    void create(vk::Device logicalDevice, vk::DescriptorPool pool, const std::vector<DescriptorSetLayout::ConstPtr> &layout, const std::vector<Buffer::ConstPtr> &buffers);
    /// @brief Make sure DescriptorPool can use create().
    //friend DescriptorSet::Ptr DescriptorPool::createDescriptorSet(const std::vector<DescriptorSetLayout::ConstPtr> &layouts, const std::vector<Buffer::ConstPtr> &buffers);

private:
    vk::DescriptorSet m_set = nullptr;
    vk::DescriptorPool m_pool = nullptr;
};

/// @brief A descriptor pool from which we allocate descriptor sets.
class DescriptorPool: public Resource
{
public:
    RESOURCE_FUNCTIONS_H(DescriptorPool)

    /// @brief Create descriptor pool.
    void create(vk::Device logicalDevice);

    /// @brief Create descriptor pool.
    DescriptorSet::Ptr createDescriptorSet(const std::vector<DescriptorSetLayout::ConstPtr> &layouts, const std::vector<Buffer::ConstPtr> &buffers);

    /// @brief Get buffer handle.
    const vk::DescriptorPool pool() const;

private:
    vk::DescriptorPool m_pool = nullptr;
};

}
