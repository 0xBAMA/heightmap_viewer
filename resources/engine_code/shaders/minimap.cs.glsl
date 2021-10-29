#version 430 core

// shader invocation details
layout( local_size_x = 64, local_size_y = 1, local_size_z = 1 ) in;

// texture working set
layout( binding = 0, rgba8ui ) uniform uimage2D renderTexture;
layout( binding = 1, rgba8ui ) uniform uimage2D minimapTexture;
layout( binding = 2, rgba8ui ) uniform uimage2D heightmap;
layout( binding = 3, rgba8ui ) uniform uimage2D colormap;

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

// need to figure out which parts of this are going to be static, and which parts are dynamic

// need masked access to the heightmap - circular mask for the heightmap
// center point being located some distance "in front of" the viewers position - along the direction vector


void drawVerticalLine( const uint x, const float yBottom, const float yTop, const uvec4 col ){
  // // this method is too general - need to break on yTop < yBottom - maybe good for another application?
  // uint yMax = imageSize( renderTexture ).y;
  // y1 = clamp( y1, 0, yMax );
  // y2 = clamp( y2, 0, yMax );
  // for( uint y = min( y1, y2 ); y <= max( y1, y2 ); y++ ){
  //   imageStore( renderTexture, ivec2( x, y ), col );
  // }

  const int yMin = clamp( int( yBottom ), 0, imageSize( renderTexture ).y );
  const int yMax = clamp( int(  yTop  ), 0, imageSize( renderTexture ).y );
  if( yMin > yMax ) return;

  for( int y = yMin; y < yMax; y++ ){
    imageStore( minimapTexture, ivec2( x, y ), col );
  }
}

bool insideMask(ivec2 queryLocation){
  return distance( vec2(queryLocation), viewPosition ) < 100.;
  // return true;
}

uint heightmapReference(ivec2 location){
  if( insideMask( location ) ){
    if( distance( location, viewPosition) < 1.618){
      return imageLoad( heightmap, location ).r + 32;
    }else{
      return imageLoad( heightmap, location ).r;
    }
  }else{
    return 0;
  }
}

uvec4 colormapReference(ivec2 location){
  if( insideMask( location ) ){
    if( distance( location, viewPosition) < 1.618){
      return uvec4( 255, 0, 0, 255 );
    }else{
      return imageLoad( colormap, location );
    }
  }else{
    return uvec4( 0 );
  }
}

mat2 rotate2D( float r ){
  return mat2( cos( r ), sin( r ), -sin( r ), cos( r ));
}

void main() {
  const float wPixels    = imageSize( renderTexture ).x;
  const float hPixels    = imageSize( renderTexture ).y;
  const int myXIndex     = int(gl_GlobalInvocationID.x);
  // float yBuffer          = hPixels - resolution.y;
  float yBuffer          = 0;

  // FoV considerations
  //   mapping [0..wPixels] to [-1..1]
  float FoVAdjust        = -1. + float( myXIndex ) * ( 2. ) / float( wPixels );

  const mat2 rotation    = rotate2D( viewAngle + FoVAdjust * FoVScalar );
  const vec2 direction   = rotation * vec2( 1, 0 );
  vec2 startCenter = viewPosition - 200 * direction;

  const vec2 forwards = rotate2D( viewAngle ) * vec2( 1, 0 );
  // const vec2 fixedDirection = direction * ( dot( direction, forwards ) / dot( forwards, forwards ) );
  const vec2 fixedDirection = direction * ( dot( forwards, forwards ) / dot( direction, forwards ) );

  // if( myXIndex > resolution.x ) return;

  // need a side-to-side adjust to give some spread - CPU side adjustment scalar needs to be added
  vec3 sideVector        = cross( vec3( forwards.x, 0., forwards.y ), vec3( 0., 1., 0. ) );
  // vec2 viewPositionLocal = viewPosition + sideVector.xz * FoVAdjust * offsetScalar;
  vec2 viewPositionLocal = startCenter + sideVector.xz * FoVAdjust * offsetScalar;

  for( float dSample = 1.0, dz = 1.0; dSample < maxDistance && yBuffer < hPixels; dSample += dz, dz += stepIncrement ){
    const ivec2 samplePosition = ivec2( viewPositionLocal + dSample * fixedDirection );
    const float heightSample    = heightmapReference(samplePosition);
    const float heightOnScreen  = (( heightSample - viewerHeight ) * ( 1. / dSample ) * heightScalar + horizonLine);

    if( heightOnScreen > yBuffer ){
      const uvec4 colorSample  = colormapReference( samplePosition );
      float distanceToColumn = sqrt( pow( dSample, 2 ) + pow( viewerHeight - heightSample, 2) );
      uint depthTerm = uint( max( 0, 255 - distanceToColumn * fogScalar ) );
      drawVerticalLine( myXIndex, yBuffer, heightOnScreen, uvec4( colorSample.xyz, depthTerm ) );
      yBuffer = uint( heightOnScreen );
    }
  }
}
