file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
add_library(vulkan-helpers STATIC ${source_files})
# target_include_directories: all code that depends on this module gets the
# provided header search path. This is contrast to include_directories, which
# propagates downward in the cmake hierarchy instead.
target_include_directories(vulkan-helpers PUBLIC $ENV{VULKAN_SDK}/Include)
