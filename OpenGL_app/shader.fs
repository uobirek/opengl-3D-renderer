#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    bool useTextures; // New flag to determine whether to use textures or uniform colors
}; 

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

#define NR_POINT_LIGHTS 1

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[2];
uniform Material material;

uniform float useFakeLighting;

float fog_maxdist = 15.0;
float fog_mindist = 5;
vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Calculate lighting effects
    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);    
           for(int i = 0; i < 2; i++)
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);    

    
    // Calculate the distance between the fragment and the camera
    float dist = length(FragPos.xyz - viewPos);  // Distance from the camera to the fragment
    
    // Adjust the fog factor to increase with distance (distance fog effect)
 float fog_factor = (fog_maxdist - dist) /
                  (fog_maxdist - fog_mindist);

    // Ensure the fog factor is between 0.0 and 1.0 (clamp it)
    fog_factor = clamp(fog_factor, 0.0, 1.0);
    
    // Apply fog by blending the lighting with the fog color based on the fog factor
    FragColor = vec4(result, 1.0);
    FragColor = mix(fog_colour, FragColor, fog_factor);  // Mix between scene color and fog color


}



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient, diffuse, specular;
    if (material.useTextures) {
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    } else {
        ambient = light.ambient * material.ambientColor;
        diffuse = light.diffuse * diff * material.diffuseColor;
        specular = light.specular * spec * material.specularColor;
    }

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient, diffuse, specular;
    if (material.useTextures) {
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    } else {
        ambient = light.ambient * material.ambientColor;
        diffuse = light.diffuse * diff * material.diffuseColor;
        specular = light.specular * spec * material.specularColor;
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient, diffuse, specular;
    if (material.useTextures) {
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    } else {
        ambient = light.ambient * material.ambientColor;
        diffuse = light.diffuse * diff * material.diffuseColor;
        specular = light.specular * spec * material.specularColor;
    }

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
