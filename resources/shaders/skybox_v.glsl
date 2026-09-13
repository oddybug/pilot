#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texture_coord;

out vec2 v_ndc;

void main() {
    gl_Position = vec4(a_pos.xy, 0.0, 1.0);
    v_ndc = a_pos.xy;
}
