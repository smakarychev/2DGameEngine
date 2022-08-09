@shaderType VERTEX

#version 450 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;

out vec3 color;

void main()
{
    gl_Position = vec4(a_position.x, a_position.y, a_position.z, 1.0);
    color = a_color;
}

@shaderType PIXEL

#version 450 core

uniform float u_time;
uniform vec3 u_anotherColor;

in vec3 color;
out vec4 FragColor;
void main()
{
    float interpVal = (sin(u_time) + 1.0f) / 2.0;
    vec3 resultColor = mix(color, u_anotherColor, interpVal);
    FragColor = vec4(resultColor, 1.0f);
}