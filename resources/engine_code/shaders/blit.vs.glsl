#version 430
in vec2 vertexPosition;

// simple passthrough
void main() {
  gl_Position = vec4(vertexPosition, 0., 1.);
}
