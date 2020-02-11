#include "vkresource.h"

#include "vkutils.h"
#include <stdexcept>

namespace vsvr
{

Resource::Resource()
{
}

Resource::Resource(Resource &&other)
{
	*this = std::move(other);
}

Resource & Resource::operator=(Resource &&other)
{
    if (&other != this)
    {
        m_isValid = std::move(other.m_isValid); other.m_isValid = false;
        m_logicalDevice = std::move(other.m_logicalDevice); other.m_logicalDevice = nullptr;
    }
    return *this;
}

Resource::~Resource()
{
    destroy();
}

void Resource::setCreated(vk::Device logicalDevice)
{
    if (m_isValid)
    {
        throw std::runtime_error("Duplicate resource creation!");
    }
    m_isValid = true;
    m_logicalDevice = logicalDevice;
}

bool Resource::isValid() const
{
    return m_isValid;
}

vk::Device Resource::logicalDevice() const
{
    return m_logicalDevice;
}

void Resource::destroy()
{
    if (m_isValid)
    {
        if (!m_logicalDevice)
        {
            throw std::runtime_error("Invalid logical device for resource!");
        }
        destroyResource();
        m_isValid = false;
    }
    m_logicalDevice = nullptr;
}

}
