#version 330 core

// Outputs from G-buffer
in vec2 TexCoords;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec; // RGB: Albedo, A: Specular intensity

// Light Structures
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 2

#define NR_SPOT_LIGHTS 1

// Uniforms
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform vec3 viewPos;

// Output
out vec4 FragColor;

// Function Prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specularStrength);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specularStrength);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specularStrength);

void main() {
    // Retrieve data from G-buffer
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec4 albedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 albedo = albedoSpec.rgb;
    float specularStrength = albedoSpec.a;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0.0);

    // Directional Light
    result += CalcDirLight(dirLight, normal, viewDir, albedo, specularStrength);

    // Point Lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir, albedo, specularStrength);
    }

    // Spot Lights
    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        result += CalcSpotLight(spotLights[i], normal, fragPos, viewDir, albedo, specularStrength);
    }

    // Final Color
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specularStrength) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * specularStrength;

    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specularStrength) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * specularStrength;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = light.ambient * albedo * attenuation;
    vec3 diffuse = light.diffuse * diff * albedo * attenuation;
    vec3 specular = light.specular * spec * attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specularStrength) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * specularStrength;

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * albedo * attenuation * intensity;
    vec3 diffuse = light.diffuse * diff * albedo * attenuation * intensity;
    vec3 specular = light.specular * spec * attenuation * intensity;

    return (ambient + diffuse + specular);
}
