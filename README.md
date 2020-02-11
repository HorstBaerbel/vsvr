# VSVR - The very simple Vulkan renderer

This a very simple and basic Vulkan rendering framework / library that abstracts some of the Vulkan objects and functions. It was supposed to make my life developing simple Vulkan applications easier, but don't expect too much from it. Use as you like according to the [MIT LICENSE](LICENSE).

# How to build?

## Prequisites

* [CMake](https://cmake.org/) for building.
* [GLFW](https://www.glfw.org/) for OS window and surface handling.
* [Vulkan SDK](https://vulkan.lunarg.com/) for Vulkan.
* [glslangValidator](https://github.com/KhronosGroup/glslang) for compiling GLSL shaders to SPIR-V.

## From the command line

* Navigate to the vsvr folder, then:

```sh
mkdir build && cd build
cmake ..
make -j $(grep -c '^processor' /proc/cpuinfo 2>/dev/null)
```

## From Visual Studio Code

* **Must**: Install the "C/C++ extension" by Microsoft.
* **Recommended**: If you want intellisense functionality install the "C++ intellisense" extension by austin.
* **Must**: Install the "CMake Tools" extension by vector-of-bool.
* Restart / Reload Visual Studio Code if you have installed extensions.
* Open the vsvr folder using "Open folder...".
* Choose "Unspecified" as your active CMake kit if asked. It then should be autodetected correctly.
* You should be able to build now using F7 and build + run using F5.

# How to use?

* Clone the library as a subdirectory to your project:  
```git clone https://github.com/HorstBaerbel/vsvr```

* Adjust your CMakeLists.txt:
  * Include the library subdirectory in your project:  
```add_subdirectory(vsvr)```
  * Add shader compilation:  
```set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")```  
```LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vsvr")```  
```include(compile_shaders REQUIRED)```  
(here your shaders will be read and written to "./shaders")
  * Add a dependency to shader compilation to your project:  
```add_dependencies(<YOUR_PROJECT> shaders)```
  * Add the library to your projects include paths:  
```target_include_directories(<YOUR_PROJECT> PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/vsvr")```
  * Make sure the library is linked to your project:  
```target_link_libraries(<YOUR_PROJECT> vsvr)```
