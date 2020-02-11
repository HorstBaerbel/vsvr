#pragma once

#include <vulkan/vulkan.hpp>
#include "glfw3.hpp"

// Define to enable debugging layer for Vulkan
// either Kronos or LunarG will be chosen, depending on availability
//#define VULKAN_VALIDATE
#ifdef VULKAN_VALIDATE
    #include "vkvalidation.h"
#endif
