# ┌──────────────────────────────────────────────────────────────────┐
# │  Projects Settings                                               │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.6.2)
project(master-thesis)

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
# Add to the INCLUDE_DIRECTORY property of the current script. If SYSTEM is
# specified, the directory is designated as a system include directory with
# the compiler.
include_directories(externals/glfw/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

# Bulk include based on a pattern: the `file` command can generate a list of
# files that match a pattern and store it in a variable.
file(GLOB_RECURSE source_files ${PROJECT_SOURCE_DIR}/src/*)
add_executable(master-thesis ${source_files})
# Specify library dependency for a target.
target_link_libraries(master-thesis glfw ${GLFW_LIBRARIES})

# Set Visual Studio startup project. I think the named property is VS_STARTUP_PROJECT
# and the given scope is that given by the CMakeLists.txt in the directory.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT master-thesis)


# ---- OTHER STUFF ------------------------------------------------------------
# Add to the list of linking directories where the linker looks for libraries.
# Note: These directories are applies to targets created after this command.
#link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Build dependencies for build order.
#add_dependencies(sharedmod libutil)
#add_dependencies(theapp sharedmod)