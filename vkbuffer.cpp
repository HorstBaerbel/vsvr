#include "vkbuffer.h"

#include "vkutils.h"
#include "vkdevice.h"
#include <cstring>
#include <numeric>

namespace vsvr
{

RESOURCE_FUNCTIONS_CPP(Buffer)

Buffer &Buffer::operator=(Buffer &&other)
{
    if (&other != this)
    {
        Resource::operator=(std::move(other));
        m_buffer = std::move(other.m_buffer); other.m_buffer = nullptr;
        m_bufferMemory = std::move(other.m_bufferMemory); other.m_bufferMemory = nullptr;
        m_bufferSize = std::move(other.m_bufferSize); other.m_bufferSize = 0;
        m_physicalDevice = std::move(other.m_physicalDevice); other.m_physicalDevice = nullptr;
        m_settings = std::move(other.m_settings); other.m_settings = Settings();
    }
    return *this;
}

void Buffer::allocateMemory(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::DeviceSize size, const Settings &settings)
{
    vk::MemoryRequirements memRequirements = logicalDevice.getBufferMemoryRequirements(m_buffer);
    m_bufferOffset = memRequirements.alignment; // make sure buffer start is aligned
    vk::MemoryAllocateInfo allocInfo(memRequirements.size, findMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, settings.properties));    
    m_bufferMemory = logicalDevice.allocateMemory(allocInfo);
    logicalDevice.bindBufferMemory(m_buffer, m_bufferMemory, m_bufferOffset);
    m_bufferSize = size;
}

void Buffer::reallocateMemory(vk::DeviceSize size)
{
    vk::DeviceSize newSize = m_bufferSize;
    // check if we need to grow the buffer
    if (size > m_bufferSize)
    {
        if (m_settings.reallocStrategy == ReallocStrategy::eGrow)
        {
            newSize = size;
        }
        else if (m_settings.reallocStrategy == ReallocStrategy::eGrowOverprovision ||
                 m_settings.reallocStrategy == ReallocStrategy::eGrowOverprovisionAndShrink)
        {
            newSize = (size * 105) / 100;
        }
        // do nothing for fixed size buffers
    }
    // check if we need to shrink the buffer
    else if (size < ((m_bufferSize * 75) / 100))
    {
        if (m_settings.reallocStrategy == ReallocStrategy::eGrowOverprovisionAndShrink)
        {
            newSize = size;
        }
    }
    // check if we need to reallocate
    if (newSize != m_bufferSize)
    {
        logicalDevice().freeMemory(m_bufferMemory);
        allocateMemory(m_physicalDevice, logicalDevice(), newSize, m_settings);
    }
}

void Buffer::create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::DeviceSize size, const Settings &settings)
{
    if (isValid())
    {
        throw std::runtime_error("Buffer already created!");
    }
    vk::BufferCreateInfo bufferInfo({}, size, settings.usage, settings.sharingMode);
    m_buffer = logicalDevice.createBuffer(bufferInfo);
    allocateMemory(physicalDevice, logicalDevice, size, settings);
    m_settings = settings;
    m_physicalDevice = physicalDevice;
    setCreated(logicalDevice);
}

vk::DeviceSize Buffer::minAligmentFor(vk::PhysicalDevice physicalDevice, vk::BufferUsageFlags usage)
{
    if (usage == vk::BufferUsageFlagBits::eStorageTexelBuffer)
    {
        return DeviceInfoCache::getProperties(physicalDevice).limits.minTexelBufferOffsetAlignment;
    }
    else if (usage == vk::BufferUsageFlagBits::eUniformBuffer)
    {
        return DeviceInfoCache::getProperties(physicalDevice).limits.minUniformBufferOffsetAlignment;
    }
    else if (usage == vk::BufferUsageFlagBits::eStorageBuffer)
    {
        return DeviceInfoCache::getProperties(physicalDevice).limits.minStorageBufferOffsetAlignment;
    }
    return 64;
}

std::pair<vk::DeviceSize, std::vector<vk::DeviceSize>> Buffer::getCombinedSizeAndOffsets(vk::PhysicalDevice physicalDevice, vk::BufferUsageFlags usage, const std::vector<RawData> &data)
{
    std::pair<vk::DeviceSize, std::vector<vk::DeviceSize>> result;
    auto minAligment = minAligmentFor(physicalDevice, usage);
    vk::DeviceSize srcOffset = 0;
    for (const auto & src : data)
    {
        // align offset to multiple of minAligment. note that minAlignment must not be a power of two...
        auto aligmentDiff = (srcOffset % minAligment);
        srcOffset = srcOffset + (aligmentDiff > 0 ? (minAligment - aligmentDiff) : 0);
        result.second.push_back(srcOffset);
        srcOffset += src.size;
    }
    // round buffer size to multiple of minAligment too, just to be sure...
    auto aligmentDiff = (srcOffset % minAligment);
    result.first = srcOffset + (aligmentDiff > 0 ? (minAligment - aligmentDiff) : 0);
    return result;
}

void Buffer::create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, const std::vector<RawData> &data, const Settings &settings)
{
    auto combinedSize = getCombinedSizeAndOffsets(physicalDevice, settings.usage, data).first;
    create(physicalDevice, logicalDevice, combinedSize, settings);
}

void Buffer::destroyResource()
{
    if (m_buffer)
    {
        logicalDevice().destroyBuffer(m_buffer);
        m_buffer = nullptr;
    }
    if (m_bufferMemory)
    {
        logicalDevice().freeMemory(m_bufferMemory);
        m_bufferMemory = nullptr;
    }
    m_bufferSize = 0;
    m_bufferOffset = 0;
}

const vk::Buffer Buffer::buffer() const
{
    return m_buffer;
}

vk::DeviceSize Buffer::offset() const
{
    return m_bufferOffset;
}

vk::DeviceSize Buffer::size() const
{
    return m_bufferSize;
}

void Buffer::update(const RawData &data)
{
    if (!isValid())
    {
        throw std::runtime_error("Buffer not yet created!");
    }
    reallocateMemory(data.size);
    if (data.size > m_bufferSize)
    {
        throw std::runtime_error("Data too big for buffer!");
    }
    void *dst = logicalDevice().mapMemory(m_bufferMemory, m_bufferOffset, data.size);
    std::memcpy(dst, data.data, data.size);
    logicalDevice().unmapMemory(m_bufferMemory);
}

void Buffer::update(const std::vector<RawData> &data)
{
    if (!isValid())
    {
        throw std::runtime_error("Buffer not yet created!");
    }
    auto combinedSizeAndOffsets = getCombinedSizeAndOffsets(m_physicalDevice, m_settings.usage, data);
    reallocateMemory(combinedSizeAndOffsets.first);
    if (combinedSizeAndOffsets.first > m_bufferSize)
    {
        throw std::runtime_error("Data too big for buffer!");
    }
    auto dst = reinterpret_cast<uint8_t *>(logicalDevice().mapMemory(m_bufferMemory, m_bufferOffset, combinedSizeAndOffsets.first));
    for (size_t i = 0; i < data.size(); i++)
    {
        const auto & src = data[i];
        auto dstOffset = combinedSizeAndOffsets.second[i];
        std::memcpy(dst + dstOffset, reinterpret_cast<const uint8_t *>(src.data) + src.offset, src.size);
    }
    logicalDevice().unmapMemory(m_bufferMemory);
}

}