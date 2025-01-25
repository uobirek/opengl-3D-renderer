#pragma once
#include <glm/glm.hpp>
#include "shader.h"

class Cube {
public:
    Cube();
    ~Cube();

    void updateModelMatrix(const glm::mat4& newModelMatrix);
    void setShaderAttributes(Shader& shader);
    void render();

private:
    void setupCube();

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    unsigned int indexCount;
    glm::mat4 modelMatrix;
};
