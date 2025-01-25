

#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform samplerCube skybox_night;
uniform float blendFactor;

void main()
{    vec4 texture1 = texture(skybox, TexCoords);
    vec4 texture2 = texture(skybox_night, TexCoords);
    vec4 finalColour = mix(texture1, texture2, blendFactor);
    FragColor = finalColour;
}

