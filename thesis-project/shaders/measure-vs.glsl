// Used when measuring recording performance. Simple pass-through without any
// transformations (we render without rasterization anyway).

#version 450

layout(constant_id = 0) const uint ALPHA_VARIANT = 0;

layout(location = 0) in vec3 i_PosL;
layout(location = 1) in vec2 i_TexC;
layout(location = 2) in vec3 i_NormL;

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec2 o_TexC;

void main() {
  // Just some silly alteration that results in different pipelines
  gl_Position = vec4(ALPHA_VARIANT == 0 ? i_PosL : i_PosL + vec3(1.0f), 1.0f);
  o_TexC = i_TexC;
}
