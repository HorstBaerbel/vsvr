#pragma once

#include "vkincludes.h"

namespace vsvr
{

/// @brief This declares all functions necessary for classes that are considered 
/// shared resources and should not be copied.
#define SHAREDRESOURCE_FUNCTIONS_H(CLASSNAME) \
using Ptr = std::shared_ptr<CLASSNAME>; \
using ConstPtr = std::shared_ptr<const CLASSNAME>; \
CLASSNAME(); \
CLASSNAME(const CLASSNAME &other) = delete; \
CLASSNAME &operator=(const CLASSNAME &other) = delete; \
CLASSNAME(CLASSNAME &&other); \
CLASSNAME &operator=(CLASSNAME &&other);

/// @brief This defines some functions necessary for classes that are considered 
/// shared resources and should not be copied.
#define SHAREDRESOURCE_FUNCTIONS_CPP(CLASSNAME) \
CLASSNAME::CLASSNAME() {} \
CLASSNAME::CLASSNAME(CLASSNAME &&other) { *this = std::move(other); }

/// @brief This declares all functions necessary for classes derived from DeviceResource
#define DEVICERESOURCE_FUNCTIONS_H(CLASSNAME) \
SHAREDRESOURCE_FUNCTIONS_H(CLASSNAME) \
protected: \
virtual void destroyResource() override; \
public:

/// @brief This defines some functions necessary for classes derived from DeviceResource
#define DEVICERESOURCE_FUNCTIONS_CPP(CLASSNAME) \
CLASSNAME::CLASSNAME() {} \
CLASSNAME::CLASSNAME(CLASSNAME &&other) : DeviceResource(std::move(other)) { *this = std::move(other); }

/// @brief A resource on a logical device, e.g. a desriptor pool or memeory pool.
class DeviceResource
{
public:
    /// @brief DeviceResource can be default constructed.
    DeviceResource();

    /// @brief DeviceResource are not copyable.
    DeviceResource(const DeviceResource &other) = delete;
    /// @brief DeviceResource are not copyable.
    DeviceResource &operator=(const DeviceResource &other) = delete;

    /// @brief DeviceResource are moveable. Invalidates other.
    DeviceResource(DeviceResource &&other);
    /// @brief DeviceResource are moveable. Invalidates other.
    DeviceResource &operator=(DeviceResource &&other);

    /// @brief Destructor. will call destroy() if m_isValid is set.
    virtual ~DeviceResource();

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
