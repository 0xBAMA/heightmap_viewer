#version 430 core

// shader invocation details
layout( local_size_x = 16, local_size_y = 16, local_size_z = 1 ) in;

// texture working set
layout( binding = 0, rgba8ui ) uniform uimage2D current;
layout( binding = 1, rgba8ui ) uniform uimage2D heightmap;
layout( binding = 2, rgba8ui ) uniform uimage2D colormap;

// current screen resolution
uniform vec2 resolution;

void main() {

}
