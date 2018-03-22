#version 450

layout(location = 0) in vec2 i_Pos;
layout(location = 1) in vec2 i_TexC;
layout(location = 2) in vec4 i_Color;

layout(push_constant) uniform TransformParameters {
  vec2 g_scale;
  vec2 g_translate;
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec2 o_TexC;

void main() {
  o_Color = i_Color;
  o_TexC = i_TexC;
  gl_Position = vec4(i_Pos * g_scale + g_translate, 0, 1);
}
