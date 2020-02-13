#include "vkbuffers.h"

#include <numeric>

namespace vsvr
{

SHAREDRESOURCE_FUNCTIONS_CPP(IndexBuffer)

IndexBuffer::IndexBuffer(MemoryPool::Ptr pool, vk::IndexType indexType, vk::DeviceSize size, const Buffer::Settings &settings)
    : m_pool(pool)
    , m_indexType(indexType)
{
    pool->createBuffer(size, settings);
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other)
{
    if (&other != this)
    {
        m_pool = std::move(other.m_pool); other.m_pool = nullptr;
        m_buffer = std::move(other.m_buffer); other.m_buffer = nullptr;
        m_indexType = std::move(other.m_indexType); other.m_indexType = vk::IndexType();
    }
    return *this;
}

void IndexBuffer::update(const RawData &data)
{
    m_pool->updateBuffer(m_buffer, data);
}

vk::IndexType IndexBuffer::indexType() const
{
    return m_indexType;
}

SHAREDRESOURCE_FUNCTIONS_CPP(VertexBuffer)

VertexBuffer::VertexBuffer(MemoryPool::Ptr pool, const std::vector<std::pair<Attribute, vk::DeviceSize>> &attributes, const Buffer::Settings &settings)
    : m_pool(pool)
{
    for (const auto & a : attributes)
    {
        m_firstBinding = std::min(m_firstBinding, a.first.vertexBinding);
        m_vertexBindings.push_back({a.first.vertexBinding, a.first.stride, a.first.inputRate});
        m_attributeBindings.push_back({a.first.attributeLocation, a.first.attributeBinding, a.first.format, 0});
        m_buffers.emplace_back(m_pool->createBuffer(a.second, settings)); 
    }
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other)
{
    if (&other != this)
    {
        m_pool = std::move(other.m_pool); other.m_pool = nullptr;
        m_buffers = std::move(other.m_buffers); other.m_buffers.clear();
        m_firstBinding = std::move(other.m_firstBinding); other.m_firstBinding = 0;
        m_vertexBindings = std::move(other.m_vertexBindings); other.m_vertexBindings.clear();
        m_attributeBindings = std::move(other.m_attributeBindings); other.m_attributeBindings.clear();
    }
    return *this;
}

void VertexBuffer::update(const std::vector<RawData> &data)
{
    m_pool->updateBuffers(buffers(), data);
}

std::vector<Buffer::Ptr> VertexBuffer::buffers() const
{
    return m_buffers;
}

std::vector<vk::DeviceSize> VertexBuffer::offsets() const
{
    std::vector<vk::DeviceSize> offsets;
    std::transform(m_buffers.cbegin(), m_buffers.cend(), std::back_inserter(offsets), [](const auto & b){ return b->offset(); });
    return offsets;
}

uint32_t VertexBuffer::firstBinding() const
{
    return m_firstBinding;
}

uint32_t VertexBuffer::bindingCount() const
{
    return static_cast<uint32_t>(m_buffers.size());
}

const std::vector<vk::VertexInputBindingDescription> &VertexBuffer::vertexBindings() const
{
    return m_vertexBindings;
}

const std::vector<vk::VertexInputAttributeDescription> &VertexBuffer::attributeBindings() const
{
    return m_attributeBindings;
}

} // namespace vsvr
