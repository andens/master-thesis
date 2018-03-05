#version 450

#extension GL_ARB_shader_draw_parameters : require

layout(row_major) uniform;
layout(row_major) buffer;

layout(location = 0) in vec3 i_PosL;
layout(location = 1) in vec2 i_TexC;
layout(location = 2) in vec3 i_NormL;

layout(push_constant) uniform CameraConstants {
  mat4 g_view;
  mat4 g_proj;
};

struct RenderJobData {
  mat4 transform;
};

layout(set = 0, location = 0) buffer RenderJobsData {
  RenderJobData g_render_jobs_data[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec2 o_TexC;
//layout(location = 1) out vec3 o_NormV;

void main() {
  gl_Position = vec4(i_PosL, 1.0f) * g_render_jobs_data[gl_DrawIDARB].transform * g_view * g_proj;

  o_TexC = i_TexC;
  //o_NormV = (vec4(i_NormL, 0.0f) * world_inv_trp * g_view).xyz;
}
