#pragma once

#include "vkincludes.h"
#include "vkresource.h"
#include "vkbuffer.h"
#include <vector>
#include <utility>
#include <string>

namespace vsvr
{

/// @brief Index buffer.
class IndexBuffer
{
public:
    SHAREDRESOURCE_FUNCTIONS_H(IndexBuffer)

    /// @brief Construct a vertex buffer on device. Call update() to fill with data.
    /// @note Make sure you set the vk::BufferUsageFlagBits::eIndexBuffer flag bit.
    IndexBuffer(MemoryPool::Ptr pool, vk::IndexType indexType, vk::DeviceSize size, const Buffer::Settings &settings);

    /// @brief Destroy vertex buffer on device. Note that the buffer will immediately be destroyed.
    ~IndexBuffer();

    /// @brief Update index data. Will reallocate depending on ReallocationStrategy passed in constructor.
    void update(const RawData &data);

    vk::IndexType indexType() const;

private:
    MemoryPool::Ptr m_pool;
    Buffer::Ptr m_buffer;
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
class VertexBuffer
{
public:
    SHAREDRESOURCE_FUNCTIONS_H(VertexBuffer)

    /// @brief Construct a vertex buffer on device. Call update() to fill with data.
    /// @note Make sure you set the vk::BufferUsageFlagBits::eVertexBuffer flag bit.
    VertexBuffer(MemoryPool::Ptr pool, const std::vector<std::pair<Attribute, vk::DeviceSize>> &attributes, const Buffer::Settings &settings);

    /// @brief Destroy vertex buffer on device. Note that the buffer will immediately be destroyed.
    ~VertexBuffer();

    /// @brief Update attribute data. Will reallocate depending on ReallocationStrategy passed in constructor.
    /// @note data must have the SAME order as used in setAttributes()!
    void update(const std::vector<RawData> &data);

    std::vector<Buffer::Ptr> buffers() const;
    std::vector<vk::DeviceSize> offsets() const;
    uint32_t firstBinding() const;
    uint32_t bindingCount() const;
    const std::vector<vk::VertexInputBindingDescription> & vertexBindings() const;
    const std::vector<vk::VertexInputAttributeDescription> & attributeBindings() const;

private:
    MemoryPool::Ptr m_pool;
    std::vector<Buffer::Ptr> m_buffers;
    uint32_t m_firstBinding = 0;
    std::vector<vk::VertexInputBindingDescription> m_vertexBindings;
    std::vector<vk::VertexInputAttributeDescription> m_attributeBindings;
};

} // namespace vsvr
