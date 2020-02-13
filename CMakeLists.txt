cmake_minimum_required(VERSION 3.1.0)

project(vsvr)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

#-------------------------------------------------------------------------------
#set up compiler flags and defines

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP") #multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP") #multi-processor compilation
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    #check for G++ 5.3+
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE G++_VERSION)
    if (G++_VERSION VERSION_LESS 5.)
            message(SEND_ERROR "You need at least G++ 5.3 to compile VSVR!")
    endif()
    #set up compiler flags for GCC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14") #support C++14
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11") #support C11
endif()

#-------------------------------------------------------------------------------
#define targets

LIST(APPEND INCLUDE_DIRECTORIES
    ./glm
)

LIST(APPEND VSVR_SOURCES
    vkbuffer.cpp
    vkbuffers.cpp
    vkdescriptor.cpp
    vkdevice.cpp
    vkpipeline.cpp
    vkrenderpass.cpp
    vkresource.cpp
    vkshader.cpp
    vkutils.cpp
    vkvalidation.cpp
    vkwindow.cpp
)

#-------------------------------------------------------------------------------
#define targets

include_directories(${INCLUDE_DIRECTORIES})
add_library(vsvr STATIC ${VSVR_SOURCES})
target_link_libraries(vsvr stdc++fs glfw vulkan)
