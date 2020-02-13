#pragma once

#include "vkresource.h"
#include "vkincludes.h"
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <memory>

namespace vsvr
{

/// @brief Struct describing a raw blob of data.
struct RawData
{
    const void *data = nullptr; // Pointer to raw data.
    vk::DeviceSize size = 0;    // Byte size of raw data.
    vk::DeviceSize offset = 0;  // Byte offset into raw data.

    template <typename T>
    RawData(const std::vector<T> &data)
        : data(data.data())
        , size(data.size)
    {
    }
};

class MemoryPool;

/// @brief Vulkan buffer object. Can be used as a vertex, index constant or uniform buffers etc.
/// Create using MemoryPool::createBuffer().
class Buffer
{
    friend MemoryPool;

public:
    enum class ReallocStrategy
    {
        eFixedSize = 0,                  // Buffer will stay fixed size and throw if updating with data too big.
        eGrow = 1,                       // Buffer wil grow as needed and never shrink.
        eGrowOverprovision = 2,          // Buffer will grow and be over-provisioned by 5%.
        eGrowOverprovisionAndShrink = 3, // Buffer will grow and be over-provisioned by 5%. It will shrink if new size is below 75%
    };

    struct Settings
    {
        vk::BufferUsageFlags usage;
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
        vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
        ReallocStrategy reallocStrategy = ReallocStrategy::eFixedSize;
    };
    
    SHAREDRESOURCE_FUNCTIONS_H(Buffer)

    /// @brief Create buffer.
    Buffer(vk::Buffer buffer, vk::DeviceSize size, vk::DeviceSize offset, const Settings &settings);

    /// @brief Get buffer handle.
    vk::Buffer buffer() const;
    /// @brief Get buffer offset.
    vk::DeviceSize offset() const;
    /// @brief Get buffer size.
    vk::DeviceSize size() const;
    /// @brief Get buffer settings.
    const Settings &settings() const;

private:
    /// @brief Update buffer with new values. MemoryPool uses this to update buffer info on reallocation.
    void updateBuffer(vk::DeviceSize newSize, vk::DeviceSize newOffset);

    vk::Buffer m_buffer = nullptr; // The buffer object
    vk::DeviceSize m_size = 0; // The size that was passed in allocation.
    vk::DeviceSize m_offset = 0; // The offset of the buffer in buffer memory.
    Settings m_settings;
};

/// @brief Simple memory allocator. Will pool types of memory that can go into the same category.
/// @note Does coalesce free memory, but currently does NOT defragment memory!
class MemoryPool: public DeviceResource
{
public:
    DEVICERESOURCE_FUNCTIONS_H(MemoryPool)

    /// @brief Get / create memory pool for device. Note that you can only have one pool per logical device.
    static MemoryPool::Ptr create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice);

    /// @brief Will allocate buffer and device memory. Call updateBuffer() to fill with data.
    Buffer::Ptr createBuffer(vk::DeviceSize size, const Buffer::Settings &settings);

    /// @brief Will allocate buffer and device memory. Call updateBuffers() to fill with data.
    std::vector<Buffer::Ptr> createBuffers(const std::vector<vk::DeviceSize> &sizes, const Buffer::Settings &settings);

    /// @brief Copy data to device memory. Depending on the ReallocStrategy it will reallocate memory if the size changes or throw.
    /// @note If the buffer is not host-visible a staging buffer will be used.
    void updateBuffer(Buffer::Ptr buffer, const RawData &data);

    /// @brief Copy multiple sets of data to device memory. Depending on the ReallocStrategy it will reallocate memory if the size changes or throw.
    /// @note If the buffer is not host-visible a staging buffer will be used.
    void updateBuffers(const std::vector<Buffer::Ptr> &buffers, const std::vector<RawData> &data);

    /// @brief Destroy buffer.
    void destroyBuffer(Buffer::Ptr buffer);

    /// @brief Destroy buffers.
    void destroyBuffers(const std::vector<Buffer::Ptr> &buffers);

    /// @brief Get the minimum alignment for a buffer type and its sub-buffers.
    /// This will return minTexelBufferOffsetAlignment, minUniformBufferOffsetAlignment, minStorageBufferOffsetAlignment,
    /// depending on the usage type. For other usage types it returns 64, which seems to be a good middle ground...
    static vk::DeviceSize minAligmentFor(vk::PhysicalDevice physicalDevice, vk::BufferUsageFlags usage);

private:
    struct Block;
    struct Page;
    struct Pool;

    struct Pool
    {
        using Iter = std::map<uint32_t, Pool>::iterator;

        uint32_t memoryTypeIndex = 0;
        std::list<Page> pages;
    };
    struct Page
    {
        using Iter = std::list<Page>::iterator;

        vk::DeviceMemory memory = nullptr;
        std::list<Block> blocks;
        Pool::Iter pool;
    };
    struct Block
    {
        using Iter = std::list<Block>::iterator;

        vk::Buffer buffer = nullptr; // Buffer handle.
        vk::DeviceSize size = 0; // Size of buffer.
        vk::DeviceSize offset = 0; // Offset of buffer in page memory.
        vk::DeviceSize requiredAlignment = 0; // Required aligment for this buffer.
        Page::Iter page;
    };
    std::map<uint32_t, Pool> m_pools; // Memory pools for a specific memory type index found via findMemoryTypeIndex()
    std::map<Buffer::Ptr, std::list<Block>::iterator> m_buffers; // for fast access to block of buffers
    vk::PhysicalDevice m_physicalDevice = nullptr;

    MemoryPool(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice);
    Page allocatePage(Pool::Iter pool, vk::DeviceSize pageSize);
    Block::Iter getFreeBlockAligned(Pool::Iter pool, vk::DeviceSize requiredSize, vk::DeviceSize requiredAlignment);
    vk::DeviceSize getOffsetShiftForAlignment(const Block::Iter block, vk::DeviceSize requiredAlignment);
    vk::DeviceSize getUsableBlockSizeForAlignment(const Block::Iter block, vk::DeviceSize requiredAlignment);
    Block::Iter allocateMemory(vk::Buffer buffer, vk::DeviceSize size, const Buffer::Settings &settings);
    Block::Iter reallocateMemory(Buffer::Ptr buffer, vk::DeviceSize size);
    void combineBlockWithFreeNeighbours(Block::Iter block);

    static const vk::DeviceSize DefaultPageSize = 64*1024*1024;
    static std::map<vk::Device, MemoryPool::Ptr> DevicePools;
};

}
