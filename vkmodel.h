#pragma once

#include "vkbuffer.h"
#include "vkpipeline.h"
#include "vkincludes.h"
#include <vector>
#include <memory>

namespace vsvr
{

/// @brief Vulkan model.
class Model
{
public:
    /// @brief Shared pointer to modifyable resource.
    using Ptr = std::shared_ptr<Model>;
    /// @brief Shared pointer to const resource.
    using ConstPtr = std::shared_ptr<const Model>;

    /// @brief Model constructor. Will allocate buffer and device memory. Call update() to fill with data.
    void Model(Buffer::SPtr m_vertexBuffer, Buffer::SPtr m_indexBuffer, Pipeline::SPtr m_pipeline);

private:
    Buffer::SPtr m_vertexBuffer;
    Buffer::SPtr m_indexBuffer;
    Pipeline::SPtr m_pipeline;
};

}
