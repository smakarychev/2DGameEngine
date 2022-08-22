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

layout (binding = 0) uniform sampler2D[32] textures;

void main()
{
    int texInd = int(vTextureIndex);
    switch (texInd)
    {
        case    -1:      finalColor  = vColor; break;
        case     0:      finalColor  = texture(textures[0],  vTexCoord * vTextureTiling) * vColor; break;
        case     1:      finalColor  = texture(textures[1],  vTexCoord * vTextureTiling) * vColor; break;
        case     2:      finalColor  = texture(textures[2],  vTexCoord * vTextureTiling) * vColor; break;
        case     3:      finalColor  = texture(textures[3],  vTexCoord * vTextureTiling) * vColor; break;
        case     4:      finalColor  = texture(textures[4],  vTexCoord * vTextureTiling) * vColor; break;
        case     5:      finalColor  = texture(textures[5],  vTexCoord * vTextureTiling) * vColor; break;
        case     6:      finalColor  = texture(textures[6],  vTexCoord * vTextureTiling) * vColor; break;
        case     7:      finalColor  = texture(textures[7],  vTexCoord * vTextureTiling) * vColor; break;
        case     8:      finalColor  = texture(textures[8],  vTexCoord * vTextureTiling) * vColor; break;
        case     9:      finalColor  = texture(textures[9],  vTexCoord * vTextureTiling) * vColor; break;
        case     10:     finalColor  = texture(textures[10], vTexCoord * vTextureTiling) * vColor; break;
        case     11:     finalColor  = texture(textures[11], vTexCoord * vTextureTiling) * vColor; break;
        case     12:     finalColor  = texture(textures[12], vTexCoord * vTextureTiling) * vColor; break;
        case     13:     finalColor  = texture(textures[13], vTexCoord * vTextureTiling) * vColor; break;
        case     14:     finalColor  = texture(textures[14], vTexCoord * vTextureTiling) * vColor; break;
        case     15:     finalColor  = texture(textures[15], vTexCoord * vTextureTiling) * vColor; break;
        case     16:     finalColor  = texture(textures[16], vTexCoord * vTextureTiling) * vColor; break;
        case     17:     finalColor  = texture(textures[17], vTexCoord * vTextureTiling) * vColor; break;
        case     18:     finalColor  = texture(textures[18], vTexCoord * vTextureTiling) * vColor; break;
        case     19:     finalColor  = texture(textures[19], vTexCoord * vTextureTiling) * vColor; break;
        case     20:     finalColor  = texture(textures[20], vTexCoord * vTextureTiling) * vColor; break;
        case     21:     finalColor  = texture(textures[21], vTexCoord * vTextureTiling) * vColor; break;
        case     22:     finalColor  = texture(textures[22], vTexCoord * vTextureTiling) * vColor; break;
        case     23:     finalColor  = texture(textures[23], vTexCoord * vTextureTiling) * vColor; break;
        case     24:     finalColor  = texture(textures[24], vTexCoord * vTextureTiling) * vColor; break;
        case     25:     finalColor  = texture(textures[25], vTexCoord * vTextureTiling) * vColor; break;
        case     26:     finalColor  = texture(textures[26], vTexCoord * vTextureTiling) * vColor; break;
        case     27:     finalColor  = texture(textures[27], vTexCoord * vTextureTiling) * vColor; break;
        case     28:     finalColor  = texture(textures[28], vTexCoord * vTextureTiling) * vColor; break;
        case     29:     finalColor  = texture(textures[29], vTexCoord * vTextureTiling) * vColor; break;
        case     30:     finalColor  = texture(textures[30], vTexCoord * vTextureTiling) * vColor; break;
        case     31:     finalColor  = texture(textures[31], vTexCoord * vTextureTiling) * vColor; break;
    }
}