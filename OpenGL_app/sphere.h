#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "Shader.h"

class Sphere {
public:
    Sphere();
    void render();
    void updateModelMatrix(const glm::mat4& modelMatrix);
    void setShaderAttributes(Shader& shader);

private:
    void setupSphere();

    unsigned int sphereVAO, sphereVBO, sphereEBO;
    unsigned int indexCount;
    glm::mat4 modelMatrix;
};

#endif
#pragma once
