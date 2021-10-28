#version 430 core

layout( binding = 0, rgba8ui ) uniform uimage2D renderOutput;
layout( binding = 1, rgba8ui ) uniform uimage2D minimapOutput;

uniform ivec2 resolution;
uniform int modeSelector;
out vec4 fragmentOutput;

void main() {
  // map sample to 0..1
	vec2 sampleLoc = gl_FragCoord.xy / vec2(resolution);

  // load uint values from the appropriate rendertexture for the pass - 0 for render output, 1 for minimap
  uvec4 loadedColor;


  if( modeSelector == 0 )
    loadedColor = imageLoad( renderOutput, ivec2( sampleLoc.xy * imageSize( renderOutput ) ) );
  else if( modeSelector == 1 )
    loadedColor = imageLoad( minimapOutput, ivec2( sampleLoc.xy * imageSize( minimapOutput ) ) );


  // map color to 0..1
  fragmentOutput = vec4( loadedColor ) / 255.;
}
