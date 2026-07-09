#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    vec2 flippedCoords = vec2(TexCoords.x, TexCoords.y);
    FragColor = texture(screenTexture, flippedCoords);
}
