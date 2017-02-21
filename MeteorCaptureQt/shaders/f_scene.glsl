#version 330

// Simple pass-through fragment shader used during the scene radiance image
// stage. No detector effects are applied at this stage.

// From the vertex shader (receives the color assigned to the vertex)
in vec3 colour_v2f;

// Output from the fragment shader (pixel colour) to screen
out vec3 colour_f2s;

void main()
{
    colour_f2s = colour_v2f;
}