#include "vkdescriptor.h"

#include "vkutils.h"

namespace vsvr
{

RESOURCE_FUNCTIONS_CPP(DescriptorSetLayout)

DescriptorSetLayout &DescriptorSetLayout::operator=(DescriptorSetLayout &&other)
{
    if (&other != this)
    {
        Resource::operator=(std::move(other));
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

RESOURCE_FUNCTIONS_CPP(DescriptorSet)

DescriptorSet & DescriptorSet::operator=(DescriptorSet &&other)
{
    if (&other != this)
	{
		Resource::operator=(std::move(other));
        m_sets = std::move(other.m_sets); other.m_sets = nullptr;
        m_pool = std::move(other.m_pool); other.m_pool = nullptr;
	}
	return *this;
}

void DescriptorSet::create(vk::Device logicalDevice, vk::DescriptorPool pool, const std::vector<DescriptorSetLayout::ConstPtr> &layout, const std::vector<Buffer::ConstPtr> &buffers)
{
    setCreated(logicalDevice);
}

void DescriptorSet::destroyResource()
{
    logicalDevice().freeDescriptorSets(m_pool, 1, &m_set);
    m_set = nullptr;
    m_pool = nullptr;
}

const vk::DescriptorSet DescriptorSet::set() const
{
    return m_set;
}

void DescriptorSet::update(const std::vector<void *> &data, std::vector<size_t> &size)
{

}

void DescriptorSet::bind(vk::CommandBuffer commandBuffer, uint32_t firstBinding = 0)
{

}

//-------------------------------------------------------------------------------------------------

RESOURCE_FUNCTIONS_CPP(DescriptorPool)

} // namespace vsvr