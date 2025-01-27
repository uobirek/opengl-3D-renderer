#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"

class Skybox {
public:
    Skybox(Shader& shader);
    ~Skybox();

    void render(const glm::mat4& view, const glm::mat4& projection, int newTime, float deltaTime);

private:
    unsigned int dayCubemapTexture;
    unsigned int nightCubemapTexture;
    unsigned int skyboxVAO, skyboxVBO;
    Shader& skyboxShader;

    float rotation = 0.0f;
    int skyboxTime = 0;

    unsigned int loadCubemap(const std::vector<std::string>& faces);
    float bindTextures() ;

    const float ROTATE_SPEED = 10.0f;
    const std::vector<std::string> dayFaces = {
        "skybox/right.jpg", "skybox/left.jpg", "skybox/top.jpg",
        "skybox/bottom.jpg", "skybox/front.jpg", "skybox/back.jpg"
    };

    const std::vector<std::string> nightFaces = {
        "skybox/nightRight.jpg", "skybox/nightLeft.jpg", "skybox/nightTop.jpg",
        "skybox/nightBottom.jpg", "skybox/nightFront.jpg", "skybox/nightBack.jpg"
    };
};

#endif
