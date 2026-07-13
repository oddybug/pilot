#version 460 core

in vec3 normal;
in vec2 text_coord;

out vec4 frag_color;

uniform sampler2D texture_f;

// TODO: Change for uniforms atleast.
vec3 light_dir = vec3(-0.2, -1.0, -0.3);
vec3 light_color = vec3(1.0, 1.0, 1.0);

void main() {
   // float ambient_strength = 0.2;
   // vec3 ambient = ambient_strength * light_color;

   // vec3 norm = normalize(normal);
   // vec3 light_direction = normalize(-light_dir);

   // float diff = max(dot(norm, light_direction), 0.0);
   // vec3 diffuse = diff * light_color;

   // vec4 result = vec4((ambient + diffuse), 1.0);
    frag_color = texture(texture_f, text_coord); //frag_color_v;
}



