@shaderType VERTEX

#version 450 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texcoord;
layout (location = 2) in vec4 a_color;

out vec2 vTexCoord;
out vec4 vColor;

uniform mat4 u_modelViewProjection;

void main()
{
    gl_Position = u_modelViewProjection * vec4(a_position, 1.0);
    vTexCoord = a_texcoord;
    vColor = a_color;
}

@shaderType PIXEL

#version 450 core

in vec2 vTexCoord;
in vec4 vColor;

out vec4 finalColor;

void main()
{
    finalColor = vColor;
}