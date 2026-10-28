#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;
uniform vec2 u_viewport;
uniform float u_thickness;
uniform vec3 u_color;
out vec3 f_color;
void emit_edge(vec4 p0, vec4 p1) {
    if (p0.w == 0.0 || p1.w == 0.0)
        return;
    vec2 ndc0 = p0.xy / p0.w;
    vec2 ndc1 = p1.xy / p1.w;
    vec2 s0 = (ndc0 * 0.5 + 0.5) * u_viewport;
    vec2 s1 = (ndc1 * 0.5 + 0.5) * u_viewport;
    vec2 dir = s1 - s0;
    float len = length(dir);
    if (len < 0.001)
        return;
    dir /= len;
    float t = max(u_thickness, 1.0) * 0.5;
    vec2 normal = vec2(-dir.y, dir.x) * t;
    f_color = u_color;
    vec2 s;
    vec2 ndc;
    float w;
    s = s0 - normal;
    ndc = s / u_viewport * 2.0 - 1.0;
    w = p0.w;
    gl_Position = vec4(ndc * w, p0.z, w);
    EmitVertex();
    s = s0 + normal;
    ndc = s / u_viewport * 2.0 - 1.0;
    gl_Position = vec4(ndc * w, p0.z, w);
    EmitVertex();
    s = s1 - normal;
    ndc = s / u_viewport * 2.0 - 1.0;
    w = p1.w;
    gl_Position = vec4(ndc * w, p1.z, w);
    EmitVertex();
    s = s1 + normal;
    ndc = s / u_viewport * 2.0 - 1.0;
    gl_Position = vec4(ndc * w, p1.z, w);
    EmitVertex();
    EndPrimitive();
}
void main() {
    emit_edge(gl_in[0].gl_Position, gl_in[1].gl_Position);
    emit_edge(gl_in[1].gl_Position, gl_in[2].gl_Position);
    emit_edge(gl_in[2].gl_Position, gl_in[0].gl_Position);
}
