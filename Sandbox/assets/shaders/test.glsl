@shaderType VERTEX

#version 450 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texcoord;

out vec2 texCoord;

uniform mat4 u_modelViewProjection;

void main()
{
    gl_Position = u_modelViewProjection * vec4(a_position, 1.0);
    texCoord = a_texcoord;
}

@shaderType PIXEL

#version 450 core

in vec2 texCoord;

out vec4 finalColor;

uniform sampler2D checkerTexture;

void main()
{
    finalColor = texture(checkerTexture, texCoord);
}