#version 430 core

// shader invocation details
layout( local_size_x = 64, local_size_y = 1, local_size_z = 1 ) in;

// texture working set
layout( binding = 0, rgba8ui ) uniform uimage2D renderTexture;
// layout( binding = 1, rgba8ui ) uniform uimage2D minimapTexture;
// layout( binding = 2, rgba8ui ) uniform uimage2D heightmap;
// layout( binding = 3, rgba8ui ) uniform uimage2D colormap;
layout( binding = 2 ) uniform sampler2DRect heightmap;
layout( binding = 3 ) uniform sampler2DRect colormap;

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

// scale the side-to-side spread
uniform float offsetScalar;

// scalar for fog distance
uniform float fogScalar;

// increase in step size as you get farther from the camera
uniform float stepIncrement; // I've seen values of 0.005 and 0.2

// adjustment for the FoV
uniform float FoVScalar;


void drawVerticalLine( const uint x, const float yBottom, const float yTop, const uvec4 col ){
  const int yMin = clamp( int( yBottom ), 0, imageSize( renderTexture ).y );
  const int yMax = clamp( int(  yTop  ), 0, imageSize( renderTexture ).y );
  if( yMin > yMax ) return;

  for( int y = yMin; y < yMax; y++ ){
    imageStore( renderTexture, ivec2( x, y ), col );
  }
}

mat2 rotate2D( float r ){
  return mat2( cos( r ), sin( r ), -sin( r ), cos( r ));
}

void main() {
  const float wPixels    = imageSize( renderTexture ).x;
  const float hPixels    = imageSize( renderTexture ).y;
  const int myXIndex     = int(gl_GlobalInvocationID.x);
  float yBuffer          = 0;

  // FoV considerations
  //   mapping [0..wPixels] to [-1..1]
  float FoVAdjust        = -1. + float( myXIndex ) * ( 2. ) / float( wPixels );

  const mat2 rotation    = rotate2D( viewAngle + FoVAdjust * FoVScalar );
  const vec2 direction   = rotation * vec2( 1, 0 );

  // need a side-to-side adjust to give some spread - CPU side adjustment scalar needs to be added
  vec3 sideVector        = cross( vec3( direction.x, 0., direction.y ), vec3( 0., 1., 0. ) );
  vec2 viewPositionLocal = viewPosition + sideVector.xz * FoVAdjust * offsetScalar;

  // for( float dSample = 1.0, dz = 1.0; dSample < maxDistance && yBuffer < hPixels; dSample += dz, dz += stepIncrement ){
  for( float dSample = 1.0, dz = 0.2; dSample < maxDistance && yBuffer < hPixels; dSample += dz, dz += stepIncrement ){
    // const ivec2 samplePosition = ivec2( viewPositionLocal + dSample * direction );
    // const float heightSample    = imageLoad( heightmap, samplePosition ).r;
    const float heightSample    = texture( heightmap, viewPositionLocal + dSample * direction ).r * 255;
    const float heightOnScreen  = (( heightSample - viewerHeight ) * ( 1. / dSample ) * heightScalar + horizonLine);

    if( heightOnScreen > yBuffer ){
      // const uvec4 colorSample  = imageLoad( colormap, samplePosition );
      const vec4 colorSample  = texture( colormap, viewPositionLocal + dSample * direction ) * 255;
      // float distanceToColumn = sqrt( pow( dSample, 2 ) + pow( viewerHeight - heightSample, 2) ); // don't like how this looked
      uint depthTerm = uint( max( 0, 255 - dSample * fogScalar ) );
      drawVerticalLine( myXIndex, yBuffer, heightOnScreen, uvec4( colorSample.xyz, depthTerm ) );
      yBuffer = uint( heightOnScreen );
    }
  }
}
