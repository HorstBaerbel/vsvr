cmake_minimum_required(VERSION 3.1.0)

# Compile GLSL shaders to SPIR-V shaders.
# Needs to have glslangValidator executable in your path.
# Usage:
# - Set SHADER_SRC_DIR to the directory your GLSL shaders will be read from:
# set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glsl")
# - Set SHADER_DST_DIR to the directory your SPIR-V shader will be written to:
# set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/spv")
# If SHADER_DST_DIR is not specified, SHADER_SRC_DIR wil be used.
# - Make sure CMake can find the module:
# LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vsve")
# find_package(compile_shaders REQUIRED)
# - Add a dependency to shaders to your executable
# add_dependencies(<YOUR_EXECUTABLE> shaders)

if (WIN32)
    find_program(GLSLANG_VALIDATOR_BINARY NAMES glslangValidator PATHS $ENV{VULKAN_SDK}/bin NO_CMAKE_FIND_ROOT_PATH)
elseif(UNIX)
    find_program(GLSLANG_VALIDATOR_BINARY NAMES glslangValidator PATHS /usr/bin NO_CMAKE_FIND_ROOT_PATH)
endif ()

if (GLSLANG_VALIDATOR_BINARY)
    message(STATUS "Found glslangValidator: ${GLSLANG_VALIDATOR_BINARY}")
    set(Glslang_FOUND TRUE)
else ()
    message(STATUS "Failed to find glslangValidator. You might need to install glslang-tools.")
endif()

if (SHADER_SRC_DIR)
    message(STATUS "Reading GLSL shaders from ${SHADER_SRC_DIR}")
    set(ShaderDst_FOUND TRUE)
    if (NOT(SHADER_DST_DIR))
        set(SHADER_DST_DIR "${SHADER_SRC_DIR}")
    endif()
    message(STATUS "Saving SPIR-V shaders to ${SHADER_DST_DIR}")
else ()
    message(STATUS "Please specify a SHADER_SRC_DIR that contains the GLSL shaders and optionally a SHADER_DST_DIR that will contain the compiled SPIR-V shaders.")
endif()

# list all shaders in shaders directory
file(GLOB_RECURSE GLSL_SRC_FILES
    "${SHADER_SRC_DIR}/*.vert"
    "${SHADER_SRC_DIR}/*.tesc"
    "${SHADER_SRC_DIR}/*.tese"
    "${SHADER_SRC_DIR}/*.geom"
    "${SHADER_SRC_DIR}/*.frag"
    "${SHADER_SRC_DIR}/*.comp"
    "${SHADER_SRC_DIR}/*.mesh"
    "${SHADER_SRC_DIR}/*.task"
)

# compile individual shaders to SPIRV
foreach(_shader_file ${GLSL_SRC_FILES})
    get_filename_component(_shader_name ${_shader_file} NAME_WE)
    get_filename_component(_shader_ext ${_shader_file} EXT)
    string(REPLACE "." "" _shader_ext "${_shader_ext}")
    set(_target_name "${_shader_name}_${_shader_ext}")
    set(_target_file "${SHADER_DST_DIR}/${_target_name}.spv")
    add_custom_command(
        OUTPUT ${_target_file}
        COMMAND ${GLSLANG_VALIDATOR_BINARY} -V ${_shader_file} -o ${_target_file}
        DEPENDS ${_shader_file}
    )
    list(APPEND SPIRV_BINARY_FILES ${_target_file})
endforeach(_shader_file)

add_custom_target(
    shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)
