#include "vkbuffer.h"

#include "vkutils.h"
#include "vkdevice.h"
#include <cstring>
#include <numeric>

namespace vsvr
{

SHAREDRESOURCE_FUNCTIONS_CPP(Buffer)

Buffer::Buffer(vk::Buffer buffer, vk::DeviceSize size, vk::DeviceSize offset, const Settings &settings)
    : m_buffer(buffer)
    , m_size(size)
    , m_offset(offset)
    , m_settings(settings)
{
}

vk::Buffer Buffer::buffer() const
{
    return m_buffer;
}

vk::DeviceSize Buffer::offset() const
{
    return m_offset;
}

vk::DeviceSize Buffer::size() const
{
    return m_size;
}

const Buffer::Settings &Buffer::settings() const
{
    return m_settings;
}

//-------------------------------------------------------------------------------------------------

std::map<vk::Device, MemoryPool::Ptr> MemoryPool::DevicePools;

MemoryPool::Ptr MemoryPool::create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice)
{
    // try to find existing pool
    auto pool = DevicePools.find(logicalDevice);
    if (pool != DevicePools.cend())
    {
        return pool->second;
    }
    // else create new one
    auto newPool = std::shared_ptr<MemoryPool>(new MemoryPool(physicalDevice, logicalDevice));
    DevicePools[logicalDevice] = newPool;
    return newPool;
}

DEVICERESOURCE_FUNCTIONS_CPP(MemoryPool)    

MemoryPool::MemoryPool(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice)
    : DeviceResource()
{
    m_physicalDevice = physicalDevice;
    setCreated(logicalDevice);
}

MemoryPool &MemoryPool::operator=(MemoryPool &&other)
{
    if (&other != this)
    {
        DeviceResource::operator=(std::move(other));
        m_pools = std::move(other.m_pools); other.m_pools.clear();
        m_buffers = std::move(other.m_buffers); other.m_buffers.clear();
        m_physicalDevice = std::move(other.m_physicalDevice); other.m_physicalDevice = nullptr;
    }
    return *this;
}

vk::DeviceSize MemoryPool::minAligmentFor(vk::PhysicalDevice physicalDevice, vk::BufferUsageFlags usage)
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

Buffer::Ptr MemoryPool::createBuffer(vk::DeviceSize size, const Buffer::Settings &settings)
{
    vk::BufferCreateInfo bufferInfo({}, size, settings.usage, settings.sharingMode);
    auto buffer = logicalDevice().createBuffer(bufferInfo);
    auto blockIt = allocateMemory(buffer, size, settings);
    blockIt->buffer = buffer;
    logicalDevice().bindBufferMemory(buffer, blockIt->page->memory, blockIt->offset);
    auto sharedBuffer = std::make_shared<Buffer>(buffer, blockIt->size, blockIt->offset, settings);
    m_buffers[sharedBuffer] = blockIt;
    return sharedBuffer;
}

std::vector<Buffer::Ptr> MemoryPool::createBuffers(const std::vector<vk::DeviceSize> &sizes, const Buffer::Settings &settings)
{
    std::vector<Buffer::Ptr> buffers;
    std::transform(sizes.cbegin(), sizes.cend(), std::back_inserter(buffers), [this, settings](const auto & s){ return createBuffer(s, settings); });
    return buffers;
}

MemoryPool::Page MemoryPool::allocatePage(MemoryPool::Pool::Iter pool, vk::DeviceSize pageSize)
{
    MemoryPool::Page page;
    vk::MemoryAllocateInfo allocInfo(pageSize, pool->second.memoryTypeIndex);
    page.memory = logicalDevice().allocateMemory(allocInfo);
    // add free block that spans the whole page
    page.blocks.emplace_front(Block({nullptr, pageSize, 0, 0}));
    page.pool = pool;
    return page;
}

vk::DeviceSize MemoryPool::getOffsetShiftForAlignment(const MemoryPool::Block::Iter block, vk::DeviceSize requiredAlignment)
{
    // if the offset is not zero and not aligned with the required alignment
    // the usable size of the block shrinks as we need to increase the offset...
    auto alignmentDiff = (block->offset % requiredAlignment);
    return (alignmentDiff > 0 ? (requiredAlignment - alignmentDiff) : 0);
}

vk::DeviceSize MemoryPool::getUsableBlockSizeForAlignment(const MemoryPool::Block::Iter block, vk::DeviceSize requiredAlignment)
{
    // if the offset is not zero and not aligned with the required alignment
    // the usable size of the block shrinks as we need to increase the offset...
    return block->size - getOffsetShiftForAlignment(block, requiredAlignment);
}

MemoryPool::Block::Iter MemoryPool::getFreeBlockAligned(MemoryPool::Pool::Iter pool, vk::DeviceSize requiredSize, vk::DeviceSize requiredAlignment)
{
    if (requiredSize > DefaultPageSize)
    {
        throw std::runtime_error("Allocation size too big!");
    }
    for (auto & page : pool->second.pages)
    {
        // try to find a free block
        auto block = page.blocks.begin();
        while (block != page.blocks.end())
        {
            // check if block is free
            if (!block->buffer)
            {
                auto freeBlock = block; // just for readability
                // check if the block is big enough with alignment
                const auto usableBlockSize = getUsableBlockSizeForAlignment(freeBlock, requiredAlignment);
                if (usableBlockSize >= requiredSize)
                {
                    // found a block. split it up
                    const auto offsetShift = getOffsetShiftForAlignment(freeBlock, requiredAlignment);
                    if (offsetShift > 0)
                    {
                        // if we must shift the offset we insert a free block before, so the previous block might use the memory if it expands
                        page.blocks.insert(block, Block({nullptr, offsetShift, block->offset, 0}));
                        // and shift the free block back by the same offset
                        freeBlock->offset += offsetShift;
                        freeBlock->size -= offsetShift;
                    }
                    // insert our block. note that the free blocks offset is already adjusted to the alignment we need
                    auto newBlock = page.blocks.insert(block, Block({nullptr, requiredSize, freeBlock->offset, requiredAlignment}));
                    // and shift the following free block behind back
                    freeBlock->offset += requiredSize;
                    freeBlock->size -= requiredSize;
                    return newBlock;
                }
            }
            ++block;
        }
    }
    // when we get here, we haven't found a block so we need to allocate a new page
    auto newPage = allocatePage(pool, DefaultPageSize);
    // this memory starts at offset 0 in a fresh memory object, so alignment is not an issue
    auto newBlock = newPage.blocks.insert(newPage.blocks.begin(), Block({nullptr, requiredSize, 0, requiredAlignment}));
    pool->second.pages.push_back(newPage);
    return newBlock;
}

MemoryPool::Block::Iter MemoryPool::allocateMemory(vk::Buffer buffer, vk::DeviceSize size, const Buffer::Settings &settings)
{
    // find out memory requirements and type for buffer
    vk::MemoryRequirements memRequirements = logicalDevice().getBufferMemoryRequirements(buffer);
    auto memTypeIndex = findMemoryTypeIndex(m_physicalDevice, memRequirements.memoryTypeBits, settings.properties);
    // check if memory pool for this type exists
    auto mpIt = m_pools.find(memTypeIndex);
    if (mpIt == m_pools.cend())
    {
        // no. allocate new pool
        mpIt = m_pools.insert(m_pools.end(), std::make_pair(memTypeIndex, Pool()));
        mpIt->second.memoryTypeIndex = memTypeIndex;
        mpIt->second.pages.emplace_front(allocatePage(mpIt, DefaultPageSize));
    }
    // find first free block of appropriate size and properly aligned
    return getFreeBlockAligned(mpIt, size, memRequirements.alignment);
}

void MemoryPool::combineBlockWithFreeNeighbours(Block::Iter block)
{
    auto &blocks = block->page->blocks;
    if (block != blocks.cbegin())
    {
        // not the first block, combine with previous block if free
        auto prevIt = std::prev(block);
        if (!prevIt->buffer)
        {
            block->size += prevIt->size;
            block->offset = prevIt->offset;
        }
        blocks.erase(prevIt);
    }
    // coalesce free memory with next
    if (block != blocks.cend())
    {
        // not the last block, combine with next block if free
        auto nextIt = std::next(block);
        if (!nextIt->buffer)
        {
            block->size += nextIt->size;
        }
        blocks.erase(nextIt);
    }
}

MemoryPool::Block::Iter MemoryPool::reallocateMemory(Buffer::Ptr buffer, vk::DeviceSize size)
{
    // try to find buffer and block
    auto bmIt = m_buffers.find(buffer);
    if (bmIt != m_buffers.end())
    {
        auto newSize = buffer->size();
        // check if we need to grow the buffer
        if (size > buffer->size())
        {
            if (buffer->settings().reallocStrategy == Buffer::ReallocStrategy::eGrow)
            {
                newSize = size;
            }
            else if (buffer->settings().reallocStrategy == Buffer::ReallocStrategy::eGrowOverprovision ||
                     buffer->settings().reallocStrategy == Buffer::ReallocStrategy::eGrowOverprovisionAndShrink)
            {
                newSize = (size * 105) / 100;
            }
            // do nothing for fixed size buffers
        }
        // check if we need to shrink the buffer
        else if (size < ((buffer->size() * 75) / 100))
        {
            if (buffer->settings().reallocStrategy == Buffer::ReallocStrategy::eGrowOverprovisionAndShrink)
            {
                newSize = size;
            }
        }
        auto block = bmIt->second;
        // check if we need to reallocate
        if (newSize != buffer->size())
        {
            // coalesce block with free free blocks before or behind
            combineBlockWithFreeNeighbours(block);
            // now find a new block
            auto requiredAlignment = block->requiredAlignment;
            block->buffer = nullptr;
            block->requiredAlignment = 0;
            auto newBlock = getFreeBlockAligned(block->page->pool, newSize, requiredAlignment);
            // rebind memory
            logicalDevice().bindBufferMemory(buffer->buffer(), newBlock->page->memory, newBlock->offset);
            buffer->updateBuffer(block->size, block->offset);
        }
        return block;
    }
    throw std::runtime_error("Unknown buffer!");
}

void MemoryPool::updateBuffer(Buffer::Ptr buffer, const RawData &data)
{
    auto block = reallocateMemory(buffer, data.size);
    if (data.size > buffer->size())
    {
        throw std::runtime_error("Data too big for buffer!");
    }
    void *dst = logicalDevice().mapMemory(block->page->memory, buffer->offset(), data.size);
    std::memcpy(dst, data.data, data.size);
    logicalDevice().unmapMemory(block->page->memory);
}

void MemoryPool::updateBuffers(const std::vector<Buffer::Ptr> &buffers, const std::vector<RawData> &data)
{
    for (size_t i = 0; i < buffers.size(); i++)
    {
        updateBuffer(buffers[i], data[i]);
    }
}

void MemoryPool::destroyBuffer(Buffer::Ptr buffer)
{
    auto bmIt = m_buffers.find(buffer);
    if (bmIt != m_buffers.end())
    {
        auto block = bmIt->second;
        // remove from map
        m_buffers.erase(bmIt);
        // free buffer and memory
        logicalDevice().destroyBuffer(block->buffer);
        block->buffer = nullptr;
        // coalesce free memory
        combineBlockWithFreeNeighbours(block);
    }
}

void MemoryPool::destroyBuffers(const std::vector<Buffer::Ptr> &buffers)
{
    std::for_each(buffers.cbegin(), buffers.cend(), [this](const auto & b){ return destroyBuffer(b); });
}

}