# Bulk include based on a pattern: the `file` command can generate a list of
# files that match a pattern and store it in a variable.
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
add_executable(thesis-project ${source_files})
# Specify library dependency for a target. This essentially builds a dependency
# tree so that all modules that links against this module (if it was a library)
# automatically links to its dependencies as well.
target_link_libraries(thesis-project vulkan-helpers glfw ${GLFW_LIBRARIES} directx-math)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Shader compilation                                              │
# └──────────────────────────────────────────────────────────────────┘

message(STATUS "Creating compilation commands for shaders")

macro(ADD_SHADER_COMMAND shader_file shader_stage)
  file(RELATIVE_PATH relative_source ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${shader_file})
  string(REGEX REPLACE "(.*)\.glsl" "\\1.spv" relative_spirv ${relative_source})
  set(output_spirv ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${relative_spirv})
  add_custom_command(
    OUTPUT ${output_spirv}
    #COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/"
    COMMAND $ENV{VULKAN_SDK}/Bin32/glslangValidator.exe -V -S ${shader_stage} -o "\"${output_spirv}\"" "\"${shader_file}\""
    DEPENDS ${shader_file}
  )
  list(APPEND SPIRV_FILES ${output_spirv})
endmacro()

file(GLOB_RECURSE shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl)
foreach(shader ${shaders})
  if("${shader}" MATCHES "-vs\.glsl$")
    ADD_SHADER_COMMAND("${shader}" "vert")
  elseif("${shader}" MATCHES "-hs\.glsl$")
    ADD_SHADER_COMMAND("${shader}" "tesc")
  elseif("${shader}" MATCHES "-ds\.glsl$")
    ADD_SHADER_COMMAND("${shader}" "tese")
  elseif("${shader}" MATCHES "-gs\.glsl$")
    ADD_SHADER_COMMAND("${shader}" "geom")
  elseif("${shader}" MATCHES "-ps\.glsl$")
    ADD_SHADER_COMMAND("${shader}" "frag")
  elseif("${shader}" MATCHES "-cs\.glsl$")
    ADD_SHADER_COMMAND("${shader}" "comp")
  else()
    message(WARNING "Not processed: ${shader}.")
  endif()
endforeach()

add_custom_target(shaders DEPENDS ${SPIRV_FILES})
add_dependencies(thesis-project shaders)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Symbolic link to assets                                         │
# └──────────────────────────────────────────────────────────────────┘

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/" asset_source)
file(TO_NATIVE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/" asset_destination)

# I managed to get CMake to treat the junction as output (and thus try to clean
# it) by using add_custom_command with OUTPUT instead of TARGET,
# add_custom_target with DEPENDS on ${asset_destination}, and adding that
# target as a dependency of thesis-project. The problem is that CMake can't
# remove the junction due to access denied and I can't seem to find a way to
# override remove() to use rd for that particular output artefact.

if (WIN32)
  add_custom_command(
    TARGET thesis-project POST_BUILD
    COMMAND rd ${asset_destination} 2> nul & mklink /J ${asset_destination} ${asset_source}
    DEPENDS ${asset_destination}
    COMMENT "Symbolic link: ${asset_source} => ${asset_destination}"
  )
else()
  add_custom_command(
    TARGET thesis-project POST_BUILD
    # CMAKE_COMMAND calls CMake itself with the -E flag, which tells it to do
    # some task, here creating a symbolic link.
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${asset_source} ${asset_destination}
    DEPENDS ${asset_destination}
    COMMENT "Symbolic link: ${asset_source} => ${asset_destination}"
  )
endif()
