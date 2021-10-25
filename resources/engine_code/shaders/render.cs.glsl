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

// adjustment for the FoV
uniform float FoVScalar;


void drawVerticalLine( const uint x, const int yBottom, const int yTop, const uvec4 col ){
  // // this method is too general - need to break on yTop < yBottom
  // uint yMax = imageSize( current ).y;
  // y1 = clamp( y1, 0, yMax );
  // y2 = clamp( y2, 0, yMax );
  // for( uint y = min( y1, y2 ); y <= max( y1, y2 ); y++ ){
  //   imageStore( current, ivec2( x, y ), col );
  // }

  const int yMin = clamp( yBottom, 0, imageSize( current ).y );
  const int yMax = clamp(    yTop, 0, imageSize( current ).y );

  if( yMin > yMax ) return;

  for( int y = yMin; y < yMax; y++ ){
    imageStore( current, ivec2( x, y ), col );
  }
}


mat2 rotate2D( float r ){
  return mat2( cos( r ), sin( r ), -sin( r ), cos( r ));
}

void main() {
  const uint wPixels    = imageSize( current ).x;
  const uint hPixels    = imageSize( current ).y;
  const uint myXIndex   = gl_GlobalInvocationID.x;
  uint yBuffer          = 0;

  // FoV considerations
  //   mapping [0..wPixels] to [-1..1]
  float FoVAdjust       = -1. + float( myXIndex ) * ( 2. ) / float( wPixels );
  const float sinAngle  = sin( viewAngle + FoVAdjust * FoVScalar );
  const float cosAngle  = cos( viewAngle + FoVAdjust * FoVScalar );

  // drawVerticalLine( myXIndex, 0, hPixels, uvec4(uvec3(uint( 255*abs(FoVAdjust) )), 255 ));

  const mat2 rotation   = rotate2D( viewAngle + FoVAdjust * FoVScalar );
  const vec2 direction  = rotation * vec2( 1, 0 );





// sort of working backward implementation
  for( float f = maxDistance; f > 15.; f -= 3. ){
    const ivec2 pos = ivec2(viewPosition + f * direction);
    drawVerticalLine( myXIndex, 0, int(int( heightScalar * imageLoad( heightmap, pos ).r - viewerHeight ) * (1./f)) , uvec4(imageLoad( colormap, pos ).xyz, uint((maxDistance-f)*fogScalar)));
  }






// broken forward implementation
  // for( float dSample = 15.0, dz = 1.0; dSample < maxDistance && yBuffer < hPixels; dSample += dz, dz += stepIncrement ){
  //   const ivec2 samplePosition = ivec2( viewPosition + dSample * direction );
  //   const uint heightSample    = imageLoad( heightmap, samplePosition ).r;
  //   const uint heightOnScreen  = uint(( heightSample - viewerHeight ) * ( 1. / dSample ) * heightScalar - horizonLine);
  //
  //   if( heightOnScreen > yBuffer ){
  //     const uvec4 colorSample  = imageLoad( colormap, samplePosition );
  //     drawVerticalLine( myXIndex, yBuffer, uint( heightOnScreen ), colorSample );
  //     yBuffer = uint( heightOnScreen );
  //   }
  // }






// totally broken first implementation
  // // consider the raycast operation for all non-edge pixels in the strip
  // float currentZ = 1.0;
  // float dZ       = 1.0;
  //
  // // angle is constant across this whole shader invocation, so these can be precomputed
  // const float sinAng    = sin( viewAngle );
  // const float cosAng    = cos( viewAngle );
  //
  // // ivec2 sloc = ivec2( myXIndex, viewPosition.x );
  // // drawVerticalLine(myXIndex, 0, uint( heightScalar * imageLoad( heightmap, sloc ).r), imageLoad( colormap, sloc ));
  //
  // // draw from front to back
  // for( int iterations = 0; ( currentZ < maxDistance ) && ( iterations < 20 ); iterations++ ){
  //
  //   // the two endpoints of the current slice across z - we have some more work here, as we are not traversing across the z slice
  //   const vec2 pLeft  = vec2( -cosAng * currentZ - sinAng * currentZ + viewPosition.x, sinAng * currentZ - cosAng * currentZ + viewPosition.y );
  //   const vec2 pRight = vec2( -cosAng * currentZ - sinAng * currentZ + viewPosition.x, sinAng * currentZ - cosAng * currentZ + viewPosition.y );
  //
  //   // step dx and dy, across z slice
  //   const float dx = ( pRight.x - pLeft.x ) / float( wPixels );
  //   const float dy = ( pRight.y - pLeft.y ) / float( wPixels );
  //
  //   // current shader invocation's position, across the z slice
  //   const vec2 pSample = pLeft + myXIndex * vec2( dx, dy );
  //
  //   // compute the height on the screen
  //   const uint sampleHeight  = imageLoad( heightmap, ivec2( pSample )).r;
  //   const int heightOnScreen = int(( viewerHeight - sampleHeight ) / currentZ * heightScalar + horizonLine );
  //
  //   // draw a vertical line
  //   const uvec4 sampleColor  = imageLoad( heightmap, ivec2( pSample ));
  //   drawVerticalLine( myXIndex, heightOnScreen, yBuffer, sampleColor );
  //
  //   // update buffered minimum encountered Y value
  //   yBuffer = max( heightOnScreen, yBuffer );
  //
  //   currentZ += dZ;             // move further out
  //   dZ       += stepIncrement; // step size becomes larger as you get farther away
  // }
}
