#version 430 core

// shader invocation details
layout( local_size_x = 16, local_size_y = 16, local_size_z = 1 ) in;

// texture working set
layout( binding = 2, rgba8ui ) uniform uimage2D heightmap;
layout( binding = 3, rgba8ui ) uniform uimage2D colormap;

void main() {

  uvec4 encodedNormal = imageLoad( colormap, ivec2( gl_GlobalInvocationID.xy ) );
  uint height = imageLoad( heightmap, ivec2( gl_GlobalInvocationID.xy ) ).r;

  vec3 normal = encodedNormal.xyz / 255.;
  normal -= vec3( 0.5 );

  normal = normalize( normal );
  vec3 color = vec3( dot( vec3( 0, 1, 0 ), normal ) ) * ( 1. / 255. ) * vec3( 1.618 * ( 255 - height ), 1.618 * float( height ), 1. / height );

  imageStore( colormap, ivec2( gl_GlobalInvocationID.xy ),  uvec4( uvec3( 255 * color ), 255));
  // imageStore( colormap, ivec2( gl_GlobalInvocationID.xy ),  uvec4( uvec3( height ), 255));
}
