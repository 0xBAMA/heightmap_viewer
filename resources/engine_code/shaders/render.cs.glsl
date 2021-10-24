#version 430 core

// shader invocation details
layout( local_size_x = 64, local_size_y = 1, local_size_z = 1 ) in;

// texture working set
layout( binding = 0, rgba8ui ) uniform uimage2D current;
layout( binding = 1, rgba8ui ) uniform uimage2D heightmap;
layout( binding = 2, rgba8ui ) uniform uimage2D colormap;

// current screen resolution
uniform ivec2 resolution;

// starting point for rendering
uniform vec2 viewPosition;

// viewer height
uniform int viewerHeight;

// view direction angle
uniform float viewAngle;

// maximum traversal
uniform int maxDistance;

// horizon line position
uniform int horizonLine;

// scale value for height
uniform float heightScalar;

// scalar for fog distance
uniform float fogScalar;



void main() {
  const uint wPixels   = imageSize( current ).x;
  const uint hPixels   = imageSize( current ).y;
  const uint myXIndex  = gl_GlobalInvocationID.x;

  const float sinAng   = sin( viewAngle );
  const float cosAng   = cos( viewAngle );

  uint yBuffer         = hPixels;
  const uvec4 border   = uvec4( 84, 38, 5, 255 );

  // border handling
  if( myXIndex == 0 || myXIndex == ( wPixels - 1 )){
    for( uint currentY = 0; currentY < hPixels; currentY++ )
      imageStore( current, ivec2( myXIndex, currentY ), border );
    return; // no part of this vertical strip needs the shader to continue
  }else{
    // top and bottom need single pixel
    imageStore( current, ivec2( myXIndex,           0 ), border );
    imageStore( current, ivec2( myXIndex, hPixels - 1 ), border );
  }

  // now consider the raycast operation for all non-edge pixels in the strip
  for( uint currentZ = 0; currentZ < maxDistance; currentZ++ ){

  }
}
