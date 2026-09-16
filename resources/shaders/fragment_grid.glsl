#version 460 core

in vec3 v_world;
in float v_depth;

out vec4 frag_color;

uniform vec3 u_line;
uniform float u_cell;
uniform float u_density;
uniform float u_major_every;
uniform float u_major_width;

float grid_line(vec2 xz, float cell, float width) {
    vec2 coord = xz / cell;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / (fwidth(coord) * width);
    return 1.0 - min(min(grid.x, grid.y), 1.0);
}

void main() {
    float fade = exp(-v_depth * u_density);
    float major = grid_line(v_world.xz, u_cell * u_major_every, u_major_width);
    float minor = grid_line(v_world.xz, u_cell, 1.0) * (1.0 - major);
    float alpha = clamp(major + minor / 3.0, 0.0, 1.0) * fade;
    if (alpha < 0.01)
        discard;
    frag_color = vec4(u_line, alpha);
}
