#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coord;

out vec3 normal;
out vec2 text_coord;

void main() {
    gl_Position = vec4( -1.0 + 2.0 * a_texture_coord.x, (-1.0 + 2.0 *
		    a_texture_coord.y) * -1.0, 0.0, 1.0);
    normal = a_normal;
    text_coord = a_texture_coord;

}

