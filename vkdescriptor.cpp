#include "vkdescriptor.h"

#include "vkutils.h"

namespace vsvr
{

DEVICERESOURCE_FUNCTIONS_CPP(DescriptorSetLayout)

DescriptorSetLayout &DescriptorSetLayout::operator=(DescriptorSetLayout &&other)
{
    if (&other != this)
    {
        DeviceResource::operator=(std::move(other));
        m_layout = std::move(other.m_layout); other.m_layout = nullptr;
    }
    return *this;
}

void DescriptorSetLayout::create(vk::Device logicalDevice, const std::vector<vk::DescriptorSetLayoutBinding> &bindings)
{
    if (isValid())
    {
        throw std::runtime_error("DescriptorSetLayout already created!");
    }
    vk::DescriptorSetLayoutCreateInfo descriptorLayout;
    descriptorLayout.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorLayout.pBindings = bindings.data();
    m_layout = logicalDevice.createDescriptorSetLayout(descriptorLayout);
    setCreated(logicalDevice);
}

void DescriptorSetLayout::destroyResource()
{
    logicalDevice().destroyDescriptorSetLayout(m_layout);
    m_layout = nullptr;
}

const vk::DescriptorSetLayout DescriptorSetLayout::layout() const
{
    return m_layout;
}

//-------------------------------------------------------------------------------------------------

DEVICERESOURCE_FUNCTIONS_CPP(DescriptorPool)

} // namespace vsvr