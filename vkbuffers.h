#pragma once

#include "vkincludes.h"
#include "vkresource.h"
#include "vkbuffer.h"
#include <vector>
#include <string>

namespace vsvr
{

/// @brief Index buffer.
class IndexBuffer : public Resource
{
public:
    RESOURCE_FUNCTIONS_H(IndexBuffer)

    /// @brief Construct a index buffer on device. Call update() to fill with data.
    /// @note Make sure you set the vk::BufferUsageFlagBits::eIndexBuffer flag bit.
    void create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::IndexType indexType, RawData &data, const Buffer::Settings &settings);

    /// @brief Update index data. If the byteSizes of data is bigger than 
    /// the underlying buffer, this will re-allocate the buffer!
    void update(const RawData &data);

    vk::IndexType indexType() const;

private:
    Buffer m_buffer;
    vk::IndexType m_indexType;
};

/// @brief Struct describing a vertex attribute for non-interleaved usage.
struct Attribute
{
    std::string name;
    uint32_t vertexBinding = 0;
    uint32_t stride = 0;
    vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex;
    uint32_t attributeLocation = 0;
    uint32_t attributeBinding = 0;
    vk::Format format;
};

/// @brief Vertex buffer with non-interleaved attribute data.
class VertexBuffer : public Resource
{
public:
    RESOURCE_FUNCTIONS_H(VertexBuffer)

    /// @brief Construct a vertex buffer on device. Call update() to fill with data.
    /// @note Make sure you set the vk::BufferUsageFlagBits::eVertexBuffer flag bit.
    void create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, const std::vector<Attribute> &attributes, const std::vector<RawData> &data, const Buffer::Settings &settings);

    /// @brief Update attribute data. If the byteSizes of the combined attributes is bigger than 
    /// the underlying buffer, this will re-allocate the buffer and update the offsets!
    /// @note data must have the SAME order as used in setAttributes()!
    void update(const std::vector<RawData> &data);

    const std::vector<VkDeviceSize> & offsets() const;
    uint32_t firstBinding() const;
    uint32_t bindingCount() const;
    const std::vector<vk::VertexInputBindingDescription> & vertexBindings() const;
    const std::vector<vk::VertexInputAttributeDescription> & attributeBindings() const;

private:
    void updateOffsets();
    void updateBindingInfo();

    struct AttributeInternal
    {
        Attribute info;
        RawData data;
    };

    Buffer m_buffer;
    std::vector<AttributeInternal> m_attributes;
    std::vector<vk::DeviceSize> m_offsets;
    uint32_t m_firstBinding = 0;
    std::vector<vk::VertexInputBindingDescription> m_vertexBindings;
    std::vector<vk::VertexInputAttributeDescription> m_attributeBindings;
};

} // namespace vsvr
