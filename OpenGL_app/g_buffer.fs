#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform bool useTexture;            // Flag to toggle between texture and fixed color
uniform vec3 fixedColor;            // Fixed color to use if not using texture
uniform sampler2D texture_diffuse1; // Texture sampler for diffuse color
uniform sampler2D texture_specular1;// Texture sampler for specular strength

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);

    if (useTexture) {
        // Use textures for albedo and specular
        gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
        gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
    } else {
        // Use fixed color for albedo and a default specular strength
        gAlbedoSpec.rgb = fixedColor;
        gAlbedoSpec.a = 1.0; // Set default specular strength (adjust as needed)
    }
}
