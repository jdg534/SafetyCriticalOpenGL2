#pragma once

static constexpr char* VERTEX_SHADER = R"(
#version 330 core
// TODO get this to be OpenGL SC 2.0 complient, GPT4 generated.
layout (location = 0) in vec2 aPos;
uniform float angle;
void main() {
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, -s, s, c);
    gl_Position = vec4(rotation * aPos, 0.0, 1.0);
}
)";

static constexpr char* FRAGMENT_SHADER = R"(
#version 330 core
// TODO get this to be OpenGL SC 2.0 complient, GPT4 generated.
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";