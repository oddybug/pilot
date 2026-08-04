#version 460 core

in vec3 normal;
in vec2 text_coord;

out vec4 frag_color;

uniform sampler2D texture_f;

void main() {
    frag_color = texture(texture_f, text_coord); //frag_color_v;
}
