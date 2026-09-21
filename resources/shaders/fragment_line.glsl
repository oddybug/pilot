#version 460 core
in vec3 f_color;
out vec4 frag_color;
uniform float u_alpha;
void main() {
    frag_color = vec4(f_color, u_alpha);
}
