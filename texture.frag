#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D uTexture;
uniform float uOpacity;

void main()
{
    vec4 texColor = texture(uTexture, TexCoord);
    texColor.a *= uOpacity;
    FragColor = texColor;
}
