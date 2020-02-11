#include "vkbuffers.h"

#include <numeric>

namespace vsvr
{

RESOURCE_FUNCTIONS_CPP(IndexBuffer)

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other)
{
    if (&other != this)
    {
        Resource::operator=(std::move(other));
        m_buffer = std::move(other.m_buffer);
        m_indexType = std::move(other.m_indexType); other.m_indexType = vk::IndexType();

    }
    return *this;
}

void IndexBuffer::create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::IndexType indexType, RawData &data, const Buffer::Settings &settings)
{
    if (isValid())
    {
        throw std::runtime_error("Index buffer already created!");
    }
    m_buffer.create(physicalDevice, logicalDevice, data.size, settings);
    m_indexType = indexType;
    setCreated(logicalDevice);
}

void IndexBuffer::update(const RawData &data)
{
    if (!isValid())
    {
        throw std::runtime_error("Index buffer not yet created!");
    }
    m_buffer.update(data);
}

vk::IndexType IndexBuffer::indexType() const
{
    return m_indexType;
}

RESOURCE_FUNCTIONS_CPP(VertexBuffer)

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other)
{
    if (&other != this)
    {
        Resource::operator=(std::move(other));
        m_buffer = std::move(other.m_buffer);
        m_attributes = std::move(other.m_attributes); other.m_attributes.clear();
        m_offsets = std::move(other.m_offsets); other.m_offsets.clear();
        m_firstBinding = std::move(other.m_firstBinding); other.m_firstBinding = 0;
        m_vertexBindings = std::move(other.m_vertexBindings); other.m_vertexBindings.clear();
        m_attributeBindings = std::move(other.m_attributeBindings); other.m_attributeBindings.clear();
    }
    return *this;
}

void VertexBuffer::create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, const std::vector<Attribute> &attributes, const std::vector<RawData> &data, const Buffer::Settings &settings)
{
    if (isValid())
    {
        throw std::runtime_error("Vertex buffer already created!");
    }
    std::transform(attributes.cbegin(), attributes.cend(), std::back_inserter(m_attributes), [](const auto & a){ return AttributeInternal({a, RawData()}); });
    m_buffer.create(physicalDevice, logicalDevice, data, settings);
    setCreated(logicalDevice);
}

void VertexBuffer::update(const std::vector<RawData> &data)
{
    if (!isValid())
    {
        throw std::runtime_error("Vertex buffer not yet created!");
    }
    m_buffer.update(data);
}

void VertexBuffer::destroyResource()
{
    m_buffer.destroy();
    m_attributes.clear();
    m_offsets.clear();
    m_firstBinding = 0;
    m_vertexBindings.clear();
    m_attributeBindings.clear();
}

void VertexBuffer::updateOffsets()
{
    m_offsets.clear();
    std::transform(m_attributes.cbegin(), m_attributes.cend(), std::back_inserter(m_offsets), [](const auto & a){ return a.data.offset; });
}

void VertexBuffer::updateBindingInfo()
{
    m_firstBinding = 0;
    m_vertexBindings.clear();
    m_attributeBindings.clear();
    for (const auto & a : m_attributes)
    {
        m_firstBinding = std::min(m_firstBinding, a.info.vertexBinding);
        m_vertexBindings.push_back({a.info.vertexBinding, a.info.stride, a.info.inputRate});
        m_attributeBindings.push_back({a.info.attributeLocation, a.info.attributeBinding, a.info.format, 0});
    }
}

const std::vector<VkDeviceSize> &VertexBuffer::offsets() const
{
    return m_offsets;
}

uint32_t VertexBuffer::firstBinding() const
{
    return m_firstBinding;
}

uint32_t VertexBuffer::bindingCount() const
{
    return static_cast<uint32_t>(m_attributes.size());
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
