#pragma once

#include "vkincludes.h"

namespace vsvr
{

/// @brief This declares all functions necessary for classes derived from Resource
#define RESOURCE_FUNCTIONS_H(CLASSNAME) \
using Ptr = std::shared_ptr<CLASSNAME>; \
using ConstPtr = std::shared_ptr<const CLASSNAME>; \
CLASSNAME(); \
CLASSNAME(const CLASSNAME &other) = delete; \
CLASSNAME &operator=(const CLASSNAME &other) = delete; \
CLASSNAME(CLASSNAME &&other); \
CLASSNAME &operator=(CLASSNAME &&other); \
protected: \
virtual void destroyResource() override; \
public:

/// @brief This defines some functions necessary for classes derived from Resource
#define RESOURCE_FUNCTIONS_CPP(CLASSNAME) \
CLASSNAME::CLASSNAME() {} \
CLASSNAME::CLASSNAME(CLASSNAME &&other) : Resource(std::move(other)) { *this = std::move(other); }

class Resource
{
public:
    /// @brief Resources can be default constructed.
    Resource();

    /// @brief Resources are not copyable.
    Resource(const Resource &other) = delete;
    /// @brief Resources are not copyable.
    Resource &operator=(const Resource &other) = delete;

    /// @brief Resources are moveable. Invalidates other.
    Resource(Resource &&other);
    /// @brief Resources are not moveable. Invalidates other.
    Resource &operator=(Resource &&other);

    /// @brief Destructor. will call destroy() if m_isValid is set.
    virtual ~Resource();

    /// @brief Call this from a derived class to see if the resource can be used.
    bool isValid() const;

    /// @brief Destroy resource.
    void destroy();

protected:
    /// @brief Call this from a derived class if the resource was properly created.
    /// Will set m_isValid = true;
    /// Will set m_logicalDevice = logicalDevice;
    void setCreated(vk::Device logicalDevice);
    /// @brief Call this from a derived class to get the device the resource is on.
    vk::Device logicalDevice() const;

    /// @brief Reimplement this in derived classes to do the actual destruction work.
    virtual void destroyResource() = 0;

private:
    bool m_isValid = false; ///!< True if resource created and usable and must be destroyed.
    vk::Device m_logicalDevice = nullptr; ///!< Device the resource is created on.
};

}
