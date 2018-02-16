#version 450

layout(location = 0) in vec2 i_TexC;
//layout(location = 1) in vec3 i_NormV;

layout(location = 0) out vec4 o_Albedo;

void main() {
  o_Albedo = vec4(i_TexC, 0.0f, 1.0f);
}
