#version 450

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec2 o_TexC;

void main() {
  // I use left handed convention. Point thumb towards yourself: front face
  // is clockwise.
  gl_Position.x = (gl_VertexIndex == 1) ? 3.0f : -1.0f;
  gl_Position.y = (gl_VertexIndex == 2) ? 3.0f : -1.0f;
  gl_Position.zw = vec2(1.0f);

  o_TexC = gl_Position.xy * vec2(0.5f, 0.5f) + 0.5f;
}
