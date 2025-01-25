#ifndef LIGHTING_H
#define LIGHTING_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h" 
#include "Camera.h"

class Lighting {
public:
    Lighting();
    void setLightingUniforms(Shader& lightingShader, const Camera& camera, int newTime, glm::vec3 spotlightPosition, glm::vec3 spotlightDirection);
        void updateDirectionalLight(Shader& lightingShader);
    void drawLightCubes(Shader& lightCubeShader, const glm::mat4& view, const glm::mat4& projection);
    void setPointLightPositions(const std::vector<glm::vec3>& positions);

private:
    std::vector<glm::vec3> pointLightPositions;
    glm::vec3 lightCubeColor = glm::vec3(1.0f, 1.0f, 1.0f);  

    unsigned int lightCubeVAO = 0, lightCubeVBO = 0;
    int skyboxTime = 0;
};

#endif
