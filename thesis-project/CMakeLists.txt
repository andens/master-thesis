# Bulk include based on a pattern: the `file` command can generate a list of
# files that match a pattern and store it in a variable.
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
add_executable(thesis-project ${source_files})
# Specify library dependency for a target. This essentially builds a dependency
# tree so that all modules that links against this module (if it was a library)
# automatically links to its dependencies as well.
target_link_libraries(thesis-project vulkan-helpers glfw ${GLFW_LIBRARIES})

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
