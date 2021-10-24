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


void main() {
  const uint wPixels   = imageSize( current ).x;
  const uint hPixels   = imageSize( current ).y;
  const uint myXIndex  = gl_GlobalInvocationID.x;

  const float sinAng   = sin( viewAngle );
  const float cosAng   = cos( viewAngle );

  uint yBuffer         = hPixels;

  // border handling
  if( myXIndex == 0 || myXIndex == ( wPixels - 1 )){
    for( uint currentY = 0; currentY < hPixels; currentY++ ) // handles edges
      imageStore( current, ivec2( myXIndex, currentY ), uvec4( 84, 38, 5, 255 ));
    return;
  }else{
    // top and bottom, single pixel
    imageStore( current, ivec2( myXIndex,           0 ), uvec4( 84, 38, 5, 255 ));
    imageStore( current, ivec2( myXIndex, hPixels - 1 ), uvec4( 84, 38, 5, 255 ));
  }

  // now consider the raycast operation
  for( uint currentZ = 0; currentZ < maxDistance; currentZ++ ){

  }

  // imageStore( current, ivec2( myXIndex, currentY ), uvec4( 0 ));
}
