# Incremental draw call recording in Vulkan

In many scenes, what exists during one frame is very likely to exist in the next one as well. Translated to draw calls, there's a good chance that a call will be redundantly recorded in subsequent frames. This repository contains the implementation for my master thesis that is based on the observation about redundant draw calls. It records incrementally by exploiting manually maintained indirect draw calls and evaluates the performance impact of recording only those calls that have changed, reusing what has been previously recorded. Two solutions to incremental recording are provided. The first leverages the multi-draw indirect feature (MDI). The other uses the experimental `VK_NVX_device_generated_commands` extension available with modern graphics hardware, which also supports switching pipelines indirectly.

# Compilation
Without having double-checked it, you need CMake to generate project files and a C++11(14?) capable compiler to build. You likely need the LunarG Vulkan SDK to find the Vulkan header (v1.0.x). Builds on Windows only due to some minor OS specific parts. You need an NVIDIA GPU that supports the extension. See [gpuinfo.org](https://vulkan.gpuinfo.org/listreports.php?extension=VK_NVX_device_generated_commands) for hardware support (generally Kepler, Maxwell, or Pascal architecture). 

# Disclaimer about code quality
Considering that this is an isolated endeavour, generality is not of great concern and as such, things are hardcoded and general structure could be better.

# Disclaimer about the thesis itself
This is mostly proof-of-concept regarding incremental recording. It aims to answer viability concerns, and does not attempt to provide answers to how it should best be implemented. That is a matter for future work if incremental recording seems worth persuing further.
