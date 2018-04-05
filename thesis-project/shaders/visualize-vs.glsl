// Vertex shader used when visualizing the scene. There is a slight difference
// between how regular recording and MDI is visualized compared to DGC. The
// former two use gl_BaseInstanceARB from VK_KHR_shader_draw_parameters to get
// an object-specific index passed via the first instance draw call parameter.
// For MDI it would also work fine to use gl_DrawIDARB if the matrices have the
// same order as the draw calls, but base instance is used because it works the
// same for regular recording. However, DGC does not seem to work with this
// extension. My workaround is to pass the index as a push constant because
// that can be generated for each draw call.

#version 450

#extension GL_ARB_shader_draw_parameters : require

layout(constant_id = 0) const uint USING_DGC = 0;

layout(row_major) uniform;
layout(row_major) buffer;

layout(location = 0) in vec3 i_PosL;
layout(location = 1) in vec2 i_TexC;
layout(location = 2) in vec3 i_NormL;

layout(push_constant) uniform CameraConstants {
  mat4 g_view;
  mat4 g_proj;
  // This parameter is only used by DGC to work around the issue of not being
  // able to use VK_KHR_shader_draw_parameters.
  uint g_transform_index;
};

struct RenderJobData {
  mat4 transform;
};

layout(set = 0, binding = 0) buffer RenderJobsData {
  RenderJobData g_render_jobs_data[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec2 o_TexC;

void main() {
  // gl_BaseInstanceARB does not seem to work with DGC. Neither does gl_BaseVertexARB
  // from the same extension.
  uint drawcall_index = USING_DGC == 1 ? g_transform_index : gl_BaseInstanceARB;

  gl_Position = vec4(i_PosL, 1.0f) * g_render_jobs_data[drawcall_index].transform * g_view * g_proj;

  o_TexC = i_TexC;
}
