@shaderType VERTEX

#version 450 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texcoord;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(a_position.x, a_position.y, a_position.z, 1.0);
    texCoord = a_texcoord;
}

@shaderType PIXEL

#version 450 core

in vec2 texCoord;

// Output fragment color
out vec4 finalColor;

uniform vec2 u_c;                 // c.x = real, c.y = imaginary component. Equation done is z^2 + c
uniform float u_zoom;             // Zoom of the scale.
uniform vec2 u_screenDims;  
uniform vec2 u_offset; 

const int MAX_ITERATIONS = 128; // Max iterations to do.

// Square a complex number
vec2 ComplexSquare(vec2 z)
{
    return vec2(
        z.x * z.x - z.y * z.y,
        z.x * z.y * 2.0
    );
}

// Convert Hue Saturation Value (HSV) color into RGB
vec3 Hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    vec3 truecolor = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    return vec3(truecolor.y, truecolor.z, truecolor.x);
}

void main()
{
    vec2 fragTexCoord = texCoord;

    vec2 z = vec2((fragTexCoord.x + u_offset.x/u_screenDims.x)*2.5/u_zoom, (fragTexCoord.y + u_offset.y/u_screenDims.y)*1.5/u_zoom);

    int iterations = 0;
    for (iterations = 0; iterations < MAX_ITERATIONS; iterations++)
    {
        z = ComplexSquare(z) + u_c;  // Iterate function

        if (dot(z, z) > 4.0) break;
    }

    z = ComplexSquare(z) + u_c;
    z = ComplexSquare(z) + u_c;

    // This last part smooths the color (again see link above).
    float smoothVal = float(iterations) + 1.0 - (log(log(length(z)))/log(2.0));

    // Normalize the value so it is between 0 and 1.
    float norm = smoothVal/float(MAX_ITERATIONS);

    // If in set, color black. 0.999 allows for some float accuracy error.
    if (norm > 0.999) finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    else finalColor = vec4(Hsv2rgb(vec3(norm, 0.8, 1.0)), 1.0);
}
