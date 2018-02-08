# Bulk include based on a pattern: the `file` command can generate a list of
# files that match a pattern and store it in a variable.
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
add_executable(thesis-project ${source_files})
# Specify library dependency for a target. This essentially builds a dependency
# tree so that all modules that links against this module (if it was a library)
# automatically links to its dependencies as well.
target_link_libraries(thesis-project vulkan-helpers glfw ${GLFW_LIBRARIES})
