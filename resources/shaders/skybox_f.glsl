#version 460 core

in vec2 v_ndc;

out vec4 frag_color;

uniform vec3 u_top;
uniform vec3 u_bottom;
uniform float u_morph;
uniform mat4 u_view;
uniform float u_tan_half_fov;
uniform float u_aspect;

void main() {
    vec3 vd = normalize(vec3(v_ndc.x * u_tan_half_fov * u_aspect,
                             v_ndc.y * u_tan_half_fov, -1.0));
    vec3 wd = transpose(mat3(u_view)) * vd;
    float t = clamp(wd.y * 0.5 + 0.5, 0.0, 1.0);
    float k0 = t;
    float k1 = t * t * (3.0 - 2.0 * t);
    float k2 = t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    float s = clamp(u_morph, 0.0, 1.0);
    float k = mix(mix(k0, k1, clamp(s * 2.0, 0.0, 1.0)), k2,
                  clamp(s * 2.0 - 1.0, 0.0, 1.0));
    frag_color = vec4(mix(u_bottom, u_top, k), 1.0);
}
