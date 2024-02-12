#version 450
#pragma shader_stage(vertex)

vec2 positions[3] = {
    {    0, -.5f },
    { -.5f,  .5f },
    {  .5f,  .5f }
};

vec3 colors[3] = {
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f }
};

layout (location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
    fragColor = colors[gl_VertexIndex];
}