file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan-helpers/*)
add_library(vulkan-helpers STATIC ${source_files} ${header_files})

# target_include_directories: all code that depends on this module gets the
# provided header search path. This is contrast to include_directories, which
# propagates downward in the cmake hierarchy instead.
# PRIVATE: Add only to vulkan-helpers.
# PUBLIC: Add to both vulkan-helpers and targets that to link to vulkan-helpers.
# INTERFACE: Do not add to vulkan-helpers, but add to targets that link vulkan-helpers.
target_include_directories(vulkan-helpers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan-helpers)
target_include_directories(vulkan-helpers PUBLIC $ENV{VULKAN_SDK}/Include)
target_include_directories(vulkan-helpers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
