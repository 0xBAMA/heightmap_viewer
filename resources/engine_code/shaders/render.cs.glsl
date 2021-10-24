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
uniform float maxDistance;

// horizon line position
uniform int horizonLine;

// scale value for height
uniform float heightScalar;

// scalar for fog distance
uniform float fogScalar;

// increase in step size as you get farther from the camera
uniform float stepIncrement; // I've seen values of 0.005 and 0.2


void drawVerticalLine( const uint x, uint y1, uint y2, const uvec4 col ){
  uint yMax = imageSize( current ).y;
  y1 = clamp( y1, 0, yMax );
  y2 = clamp( y2, 0, yMax );
  for( uint y = min( y1, y2 ); y <= max( y1, y2 ); y++ ){
    imageStore( current, ivec2( x, y ), col );
  }
}


void main() {
  const uint wPixels    = imageSize( current ).x;
  const uint hPixels    = imageSize( current ).y;
  const uint myXIndex   = gl_GlobalInvocationID.x;
  uint yBuffer          = 0;


  // consider the raycast operation for all non-edge pixels in the strip
  float currentZ = 1.0;
  float dZ       = 1.0;

  // angle is constant across this whole shader invocation, so these can be precomputed
  const float sinAng    = sin( viewAngle );
  const float cosAng    = cos( viewAngle );

  // ivec2 sloc = ivec2( myXIndex, viewPosition.x );
  // drawVerticalLine(myXIndex, 0, uint( heightScalar * imageLoad( heightmap, sloc ).r), imageLoad( colormap, sloc ));

  // draw from front to back
  for( int iterations = 0; ( currentZ < maxDistance ) && ( iterations < 20 ); iterations++ ){

    // the two endpoints of the current slice across z - we have some more work here, as we are not traversing across the z slice
    const vec2 pLeft  = vec2( -cosAng * currentZ - sinAng * currentZ + viewPosition.x, sinAng * currentZ - cosAng * currentZ + viewPosition.y );
    const vec2 pRight = vec2( -cosAng * currentZ - sinAng * currentZ + viewPosition.x, sinAng * currentZ - cosAng * currentZ + viewPosition.y );

    // step dx and dy, across z slice
    const float dx = ( pRight.x - pLeft.x ) / float( wPixels );
    const float dy = ( pRight.y - pLeft.y ) / float( wPixels );

    // current shader invocation's position, across the z slice
    const vec2 pSample = pLeft + myXIndex * vec2( dx, dy );

    // compute the height on the screen
    const uint sampleHeight  = imageLoad( heightmap, ivec2( pSample )).r;
    const int heightOnScreen = int(( viewerHeight - sampleHeight ) / currentZ * heightScalar + horizonLine );

    // draw a vertical line
    const uvec4 sampleColor  = imageLoad( heightmap, ivec2( pSample ));
    drawVerticalLine( myXIndex, heightOnScreen, yBuffer, sampleColor );

    // update buffered minimum encountered Y value
    yBuffer = max( heightOnScreen, yBuffer );

    currentZ += dZ;             // move further out
    dZ       += stepIncrement; // step size becomes larger as you get farther away
  }
}
