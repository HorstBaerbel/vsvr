#pragma once

#include "vkbuffer.h"
#include "vkresource.h"
#include "vkincludes.h"
#include <vector>
#include <memory>

namespace vsvr
{

/// @brief Shader resource binding types for a pipeline.
class DescriptorSetLayout: public DeviceResource
{
public:
    DEVICERESOURCE_FUNCTIONS_H(DescriptorSetLayout)

    /// @brief Get descriptor set layout.
    const vk::DescriptorSetLayout layout() const;

    /// @brief Create descriptor set layout.
    void create(vk::Device logicalDevice, const std::vector<vk::DescriptorSetLayoutBinding> &bindings);

private:
    vk::DescriptorSetLayout m_layout = nullptr;
};

/// @brief Shader resource binding data for a pipeline.
struct DescriptorSet
{
    std::vector<DescriptorSetLayout::ConstPtr> layouts;
    std::vector<Buffer::ConstPtr> buffers;
    uint32_t binding = 0;
};

/// @brief A descriptor pool from which we allocate descriptor sets.
class DescriptorPool: public DeviceResource
{
public:
    DEVICERESOURCE_FUNCTIONS_H(DescriptorPool)

    /// @brief Create descriptor pool.
    void create(vk::Device logicalDevice);

    /// @brief Allocate descriptor sets from pool. Call this for ALL layouts you want to use in a frame.
    /// Then use 
    std::vector<DescriptorSet> createDescriptorSets(const std::vector<DescriptorSetLayout::ConstPtr> &sets);

    /// @brief Updates descriptor sets with data from buffers.
    /// Must call createDescriptorSets first
    std::vector<DescriptorSet> update(const std::vector<DescriptorSet> &sets);

    /// @brief Reset the pool for reusing it in a different frame.
    /// This will destroy all descriptor sets.
    void reset();

private:
    struct DescriptorSetInternal
    {
        DescriptorSetLayout::ConstPtr layout;
        Buffer::ConstPtr buffer;
        vk::DescriptorSet set;
    };

    vk::DescriptorPool m_pool = nullptr;
};

}
