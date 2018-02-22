# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.6.2)
project(thesis-project)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
message(STATUS "Setting MSVC flags")
# Specify flags to the compiler. This should probably be conditional depending
# on what compiler is targeted if done properly.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")
# Another approach
#list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE STRING "" FORCE)
set(GLFW_INSTALL OFF CACHE STRING "" FORCE)
# Add subdirectory whose CMakeLists.txt will be processed.
add_subdirectory(externals/glfw)
# Include directory for those linking against glfw
target_include_directories(glfw PUBLIC externals/glfw/include)

# DirectXMath
# INTERFACE creates no build output, but may have properties and stuff.
add_library(directx-math INTERFACE)
target_include_directories(directx-math INTERFACE externals/DirectXMath/Inc)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Output directories and project parts                            │
# └──────────────────────────────────────────────────────────────────┘

# Apparently, Visual Studio at school creates Debug or Release directories
# inside the directory I set with CMAKE_RUNTIME_OUTPUT_DIRECTORY but not at
# home, causing issues with relative paths.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build")

add_subdirectory(vulkan-helpers)
add_subdirectory(thesis-project)

# Set Visual Studio startup project. I think the named property is VS_STARTUP_PROJECT
# and the given scope is that given by the CMakeLists.txt in the directory.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT thesis-project)

# ---- OTHER STUFF ------------------------------------------------------------
# Add to the list of linking directories where the linker looks for libraries.
# Note: These directories are applies to targets created after this command.
#link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Build dependencies for build order.
#add_dependencies(sharedmod libutil)
#add_dependencies(theapp sharedmod)

# https://github.com/Barthelemy/CppProjectTemplate
