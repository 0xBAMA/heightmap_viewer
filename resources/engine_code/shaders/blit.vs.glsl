#version 430
in vec2 vertexPosition;
uniform int modeSelector;

void main() {
  // modeSelector controls z value, so that minimap blends over existing output
  gl_Position = vec4( vertexPosition, ( modeSelector == 1 ) ? -0.5 : 0.5, 1. );
}
