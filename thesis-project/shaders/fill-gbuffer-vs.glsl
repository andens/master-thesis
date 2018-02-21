#version 450

layout(row_major) uniform;

layout(location = 0) in vec3 i_PosL;
layout(location = 1) in vec2 i_TexC;
layout(location = 2) in vec3 i_NormL;

layout(push_constant) uniform CameraConstants {
  mat4 g_view;
  mat4 g_proj;
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec2 o_TexC;
//layout(location = 1) out vec3 o_NormV;

void main() {
  //gl_Position = vec4(i_PosL, 1.0f);
  //gl_Position = i_PosL * g_view * g_proj;

  vec4 hard_coded_pos;
  hard_coded_pos.x = (gl_VertexIndex == 1) ? 1.0f : 0.0f;
  hard_coded_pos.y = (gl_VertexIndex == 2) ? 0.0f : 1.0f;
  hard_coded_pos.zw = vec2(1.0f);
  gl_Position = hard_coded_pos * g_view * g_proj;

  o_TexC = i_TexC;
  //o_NormV = (vec4(i_NormL, 0.0f) * world_inv_trp * g_view).xyz;
}
