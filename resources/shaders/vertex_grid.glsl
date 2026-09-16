#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

out vec3 v_world;
out float v_depth;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 world = model * vec4(a_pos, 1.0);
    v_world = world.xyz;
    vec4 view_pos = view * world;
    v_depth = -view_pos.z;
    gl_Position = projection * view_pos;
}
