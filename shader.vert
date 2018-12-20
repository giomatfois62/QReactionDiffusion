

#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;
layout (location = 3) in vec3 col;

uniform mat4 mvp;
uniform mat3 n;

out vec3 normal;
out vec2 texCoord;
out vec3 color;

void main(void)
{
    gl_Position = mvp * vec4(pos, 1.0);
    normal = normalize(n * norm);
    texCoord = tex;
    color = col;
}
