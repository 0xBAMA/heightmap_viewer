#version 430 core

// shader invocation details
layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;
layout( binding = 0, rgba8ui ) uniform uimage2D current;

void main(){
  imageStore( current, ivec2(gl_GlobalInvocationID.xy), uvec4( 0 ));
}
