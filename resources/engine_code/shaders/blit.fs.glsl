#version 430 core
layout( binding = 0, rgba8ui ) uniform uimage2D current;

uniform vec2 resolution;
out vec4 fragmentOutput;

void main() {
  // map sample to 0..1
	vec2  sampleLoc = gl_FragCoord.xy / resolution;

  // pull from the render texture -
  //   doing it this way decouples render texture resolution from screen resolution
  uvec4 loadedColor = imageLoad( current, ivec2( sampleLoc.xy * imageSize( current ).xy ) );

  // map color to 0..1
	fragmentOutput = vec4( loadedColor ) / 255.;

  // may want to use alpha for depth fog, as it will blend w/ clear color
  // fragmentOutput.a = 1.;
}
