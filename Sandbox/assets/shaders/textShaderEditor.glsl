@shaderType VERTEX

#version 450 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in float a_textureIndex;
layout (location = 2) in vec2 a_texcoord;
layout (location = 3) in vec2 a_textureTiling;
layout (location = 4) in vec4 a_color;

out vec2 vTexCoord;
out vec4 vColor;
out float vTextureIndex;
out vec2 vTextureTiling;

uniform mat4 u_modelViewProjection;


void main()
{
    gl_Position = u_modelViewProjection * vec4(a_position, 1.0);
    vTexCoord = a_texcoord;
    vColor = a_color;
    vTextureIndex = a_textureIndex;
    vTextureTiling = a_textureTiling;
}

@shaderType PIXEL

#version 450 core

in vec2 vTexCoord;
in vec4 vColor;
in float vTextureIndex;
in vec2 vTextureTiling;

out vec4 finalColor;

layout (binding = 0) uniform sampler2D fontAtlas;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange() {
    vec2 unitRange = vec2(4.0)/vec2(textureSize(fontAtlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(vTexCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

void main()
{
    
    vec3 msd = texture(fontAtlas, vTexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    finalColor = mix(vec4(vColor.rgb, 0.0), vColor, opacity);
}