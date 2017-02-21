#version 330

// Shader performs the perspective projection of the scene during production of
// the scene radiance texture.

// Inputs (scene geometry)
layout(location = 0) in vec4 position;   // Vertex position is bound to location 0 automatically
layout(location = 2) in vec3 normal;     // Vertex normal is bound to location 2 automatically
layout(location = 3) in vec3 colour_in;  // Vertex colour is bound to location 3 automatically

// Pass vertex colour along to next stage (fragment shader)
out vec3 colour_v2f;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

// Light source position and brightness (in eye coordinates)
struct PointLight
{
    vec3 position;
    vec3 brightness;
};

uniform PointLight sun;

void main()
{

    // Vertex position in eye coordinates
    vec4 V = modelViewMatrix * position;

    // Normal vector in eye coordinates
    vec3 N = (inverse(transpose(modelViewMatrix)) * vec4(normal,1)).xyz;
    
    // Vertex-Sun direction in eye coordinates
    vec3 L = sun.position - V.xyz;

    // Distance to light source (to apply attenuation)
    float d2 = dot(L,L);

    // Cosine of angle between vertex normal and light direction
    float cosA = dot(normalize(N), normalize(L));

    // Apply Lambert lighting equation
    //
    // colour_in represents the albedo of the object, in each colour channel
    // cosA is the Lambert reflectance function term
    // sun.brightness is the intensity of incident light in each channel
    // 
    colour_v2f  = colour_in * cosA * sun.brightness / d2;

    // Project vertex into clip space
    gl_Position = projectionMatrix * V;

    
}