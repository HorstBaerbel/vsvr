#pragma once

#include "vkresource.h"
#include "vkincludes.h"
#include <vector>
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
};

/// @brief Vulkan buffer object. Can be used as a vertex, index constant or uniform buffers etc.
/// TODO: At the moment this allocates one buffer object per Buffer. This is not optimal 
/// and should be changed to use a simple custom allocator.
class Buffer: public Resource
{
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

    RESOURCE_FUNCTIONS_H(Buffer)

    /// @brief Buffer constructor. Will allocate buffer and device memory. Call update() to fill with data.
    void create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::DeviceSize size, const Settings &settings);

    /// @brief Buffer constructor. Will allocate buffer and device memory. Call update() to fill with data.
    template<typename T>
    void create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, const std::vector<T> &data, vk::DeviceSize offset, const Settings &settings)
    {
        create(physicalDevice, logicalDevice, data.size() * sizeof(T), settings);
    }

    /// @brief Buffer constructor. Will allocate buffer and device memory. Call update() to fill with data.
    void create(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, const std::vector<RawData> &data, const Settings &settings);

    /// @brief Get buffer handle.
    const vk::Buffer buffer() const;

    /// @brief Get buffer offset.
    vk::DeviceSize offset() const;

    /// @brief Get buffer size.
    vk::DeviceSize size() const;

    /// @brief Copy data to device memory.
    void update(const RawData &data);

    /// @brief Copy data to device memory.
    template<typename T>
    void update(const std::vector<T> &data)
    {
        update(data.data(), data.size() * sizeof(T));
    }

    /// @brief Copy multiple sets of data to device memory.
    void update(const std::vector<RawData> &data);

    /// @brief Get the minimum alignment for a buffer type and its sub-buffers.
    /// This will return minTexelBufferOffsetAlignment, minUniformBufferOffsetAlignment, minStorageBufferOffsetAlignment,
    /// depending on the usage type. For other usage types it returns 64, which seems to a good middle ground...
    static vk::DeviceSize minAligmentFor(vk::PhysicalDevice physicalDevice, vk::BufferUsageFlags usage);

    // @brief Get the minimum size and sub-buffer offsets for a buffer type and its sub-buffers according to sub-buffer sizes and alignment.
    std::pair<vk::DeviceSize, std::vector<vk::DeviceSize>> getCombinedSizeAndOffsets(vk::PhysicalDevice physicalDevice, vk::BufferUsageFlags usage, const std::vector<RawData> &data);

private:
    void allocateMemory(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, vk::DeviceSize size, const Settings &settings);
    void reallocateMemory(vk::DeviceSize size);

    vk::Buffer m_buffer;
    vk::DeviceMemory m_bufferMemory;
    vk::DeviceSize m_bufferSize = 0;
    vk::DeviceSize m_bufferOffset = 0;
    vk::PhysicalDevice m_physicalDevice = nullptr;
    Settings m_settings;
};

}
