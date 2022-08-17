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

float saturate( float x ) { return clamp( x, 0.0, 1.0 ); }

vec3 inferno_quintic( float x )
{
	x = saturate( x );
	vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
	vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
	return vec3(
		dot( x1.xyzw, vec4( -0.027780558, +1.228188385, +0.278906882, +3.892783760 ) ) + dot( x2.xy, vec2( -8.490712758, +4.069046086 ) ),
		dot( x1.xyzw, vec4( +0.014065206, +0.015360518, +1.605395918, -4.821108251 ) ) + dot( x2.xy, vec2( +8.389314011, -4.193858954 ) ),
		dot( x1.xyzw, vec4( -0.019628385, +3.122510347, -5.893222355, +2.798380308 ) ) + dot( x2.xy, vec2( -3.608884658, +4.324996022 ) ) );
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
    //else finalColor = vec4(Hsv2rgb(vec3(norm, 0.8, 1.0)), 1.0);
    else finalColor = vec4(inferno_quintic(norm), 1.0);
}
