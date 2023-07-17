#version 430 core

// shader invocation details
layout( local_size_x = 64, local_size_y = 1, local_size_z = 1 ) in;

// texture working set
layout( binding = 0, rgba8ui ) uniform uimage2D renderTexture;
layout( binding = 1, rgba8ui ) uniform uimage2D minimapTexture;

// layout( binding = 2, rgba8ui ) uniform uimage2D heightmap;
// layout( binding = 3, rgba8ui ) uniform uimage2D colormap;

// layout( binding = 2 ) uniform sampler2DRect heightmap;
// layout( binding = 3 ) uniform sampler2DRect colormap;

layout( binding = 2 ) uniform sampler2D heightmap;
layout( binding = 3 ) uniform sampler2D colormap;


// current screen resolution
uniform ivec2 resolution;

// starting point for rendering
uniform vec2 viewPosition;

// viewer height
// uniform int viewerHeight;
const int viewerHeight = 300;

// how high the viewer is off the heightmap
uniform int viewerElevation = 300;

// view direction angle
uniform float viewAngle;

// maximum traversal
// uniform float maxDistance;
const float maxDistance = 400;

// horizon line position
// uniform int horizonLine;
const int horizonLine = 1000;

// scale value for height
// uniform float heightScalar;
const float heightScalar = 150;

// scale the side-to-side spread
// uniform float offsetScalar;
const float offsetScalar = 110;

// how much to move the player back
uniform float viewBump;

// minimap scale factor
uniform float minimapScalar;

// adjustment for the FoV
// uniform float FoVScalar;
const float FoVScalar = 0.275;

// need to figure out which parts of this are going to be static, and which parts are dynamic

// need masked access to the heightmap - circular mask for the heightmap
// center point being located some distance "in front of" the viewers position - along the direction vector

uvec4 backgroundColor = uvec4( 255, 0, 0, 255 );

// void drawVerticalLine( const uint x, const float yBottom, const float yTop, const uvec4 col ){
void drawVerticalLine( const uint x, const float yBottom, const float yTop, const vec4 col ){
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
    imageStore( minimapTexture, ivec2( x, y ), uvec4( col ) );
  }
}

vec2 globalForwards = vec2(0);

bool insideMask( vec2 queryLocation ){
  return distance( queryLocation, viewPosition + vec2( viewBump * globalForwards ) ) < ( 100. / minimapScalar );
  // return distance( vec2(queryLocation), viewPosition ) < 100.;
}

// uint heightmapReference(ivec2 location){
float heightmapReference( vec2 location ){
  location = vec2( ( vec2( location - viewPosition ) * ( 1. / minimapScalar ) ) + viewPosition );
  location += vec2( viewBump * globalForwards );
  if( insideMask( location ) ){
    if( distance( location, viewPosition ) < ( 1.618 / minimapScalar ) ){
      // return imageLoad( heightmap, location ).r + viewerElevation;
      return ( texture( heightmap, location / textureSize( heightmap, 0 ) ).r * 255 ) + viewerElevation;
    }else{
      // return imageLoad( heightmap, location ).r;
      return ( texture( heightmap, location / textureSize( heightmap, 0 ) ).r * 255 );
    }
  }else{
    return 0;
  }
}

// uvec4 colormapReference(ivec2 location){
vec4 colormapReference( vec2 location ){
  location = vec2( ( vec2( location - viewPosition ) * ( 1. / minimapScalar ) ) + viewPosition );
  location += vec2( viewBump * globalForwards );
  if( insideMask( location ) ){
    if( distance( location, viewPosition ) < ( 1.618 / minimapScalar ) ){
      return vec4( 255, 0, 0, 255 );
    }else if( distance( location, viewPosition + vec2( viewBump * globalForwards ) ) > ( 97. / minimapScalar ) ){
      return vec4( 0, 0, 0, 0 );
    }else{
      return texture( colormap, location / textureSize( colormap, 0 ) ) * 255;
    }
  }else{
    return vec4( 0 );
  }
}

mat2 rotate2D( float r ){
  return mat2( cos( r ), sin( r ), -sin( r ), cos( r ));
}

void main () {

  const float wPixels    = resolution.x;
  const float hPixels    = imageSize( minimapTexture ).y;
  const float myXIndex   = gl_GlobalInvocationID.x;
  float yBuffer          = 13.0f * hPixels / 24.0f;

  if ( myXIndex > wPixels ) return;

  // FoV considerations
  //   mapping [0..wPixels] to [-1..1]
  float FoVAdjust        = -1. + float( myXIndex ) * ( 2. ) / float( wPixels );

  const mat2 rotation    = rotate2D( viewAngle + FoVAdjust * FoVScalar );
  const vec2 direction   = rotation * vec2( 1, 0 );
  vec2 startCenter = viewPosition - 200 * direction;

  const vec2 forwards = globalForwards = rotate2D( viewAngle ) * vec2( 1, 0 );
  const vec2 fixedDirection = direction * ( dot( direction, forwards ) / dot( forwards, forwards ) );

  // need a side-to-side adjust to give some spread - CPU side adjustment scalar needs to be added
  vec3 sideVector        = cross( vec3( forwards.x, 0., forwards.y ), vec3( 0., 1., 0. ) );
  vec2 viewPositionLocal = startCenter + sideVector.xz * FoVAdjust * offsetScalar;

  const float dz = 0.25;
  for( float dSample = 1.0; dSample < maxDistance && yBuffer < hPixels; dSample += dz ){
    const ivec2 samplePosition = ivec2( viewPositionLocal + dSample * fixedDirection );
    const float heightSample    = heightmapReference( samplePosition );
    const float heightOnScreen  = ( ( heightSample - viewerHeight ) * ( 1.0f / dSample ) * heightScalar + horizonLine );

    if ( heightOnScreen > yBuffer ) {
      drawVerticalLine( int( myXIndex ), yBuffer, heightOnScreen, colormapReference( samplePosition ) );
      yBuffer = uint( heightOnScreen );
    }
  }
}
