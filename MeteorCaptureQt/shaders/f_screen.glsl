#version 330

// Simple pass-through fragment shader used to render quad textures.

// Texture coordinate of this fragment
in vec2 texCoord;

// Output from the fragment shader (pixel colour)
out vec3 colourOut;

uniform sampler2D tex;

void main()
{
    colourOut = texture2D(tex, texCoord).rgb;
}