#include "Skybox.h"
#include <iostream>
#include "stb_image.h"

Skybox::Skybox( Shader& shader)
    : skyboxShader(shader) {
    // Load cubemap textures
  
    dayCubemapTexture = loadCubemap(dayFaces);
    nightCubemapTexture = loadCubemap(nightFaces);

    // Skybox vertices
    float skyboxVertices[] = {
        // positions          
              -1.0f,  1.0f, -1.0f,
              -1.0f, -1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,
               1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,

              -1.0f, -1.0f,  1.0f,
              -1.0f, -1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f, -1.0f,  1.0f,

               1.0f, -1.0f, -1.0f,
               1.0f, -1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,

              -1.0f, -1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f, -1.0f,  1.0f,
              -1.0f, -1.0f,  1.0f,

              -1.0f,  1.0f, -1.0f,
               1.0f,  1.0f, -1.0f,
               1.0f,  1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f, -1.0f,

              -1.0f, -1.0f, -1.0f,
              -1.0f, -1.0f,  1.0f,
               1.0f, -1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,
              -1.0f, -1.0f,  1.0f,
               1.0f, -1.0f,  1.0f
    };

    // Setup VAO and VBO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Set shader textures
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    skyboxShader.setInt("skybox_night", 1);
}

Skybox::~Skybox() {
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
}

unsigned int Skybox::loadCubemap(const std::vector<std::string>& faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

float Skybox::bindTextures( ) {

    int texture1;
    int texture2;
    float blendFactor = 0.0f; // Default blendFactor (no blending)

    if (skyboxTime >= 0 && skyboxTime < 5000) {
        // Full night
        texture1 = nightCubemapTexture;
        texture2 = nightCubemapTexture;
        blendFactor = 0.0f;
    }
    else if (skyboxTime >= 5000 && skyboxTime < 6000) {
        // Night to Day transition
        texture1 = nightCubemapTexture;
        texture2 = dayCubemapTexture;
        blendFactor = (float)(skyboxTime - 5000) / (6000 - 5000);
    }
    else if (skyboxTime >= 6000 && skyboxTime < 21000) {
        // Full day
        texture1 = dayCubemapTexture;
        texture2 = dayCubemapTexture;
        blendFactor = 0.0f;
    }
    else if (skyboxTime >= 21000 && skyboxTime < 22000) {
        // Day to Night transition
        texture1 = dayCubemapTexture;
        texture2 = nightCubemapTexture;
        blendFactor = (float)(skyboxTime - 21000) / (22000 - 21000);
    }
    else if (skyboxTime >= 22000 && skyboxTime < 24000) {
        // Full night
        texture1 = nightCubemapTexture;
        texture2 = nightCubemapTexture;
        blendFactor = 0.0f;
    }


    // Texture binding logic (comment out if unnecessary in this function)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture2);

    return blendFactor;
}

void Skybox::render(const glm::mat4& view, const glm::mat4& projection, int newTime, float deltaTime) {
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_CLAMP);

    skyboxShader.use();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // Remove translation
    rotation += ROTATE_SPEED * deltaTime;
    this->skyboxTime = newTime;
    skyboxView = glm::rotate(skyboxView, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    skyboxShader.setMat4("view", skyboxView);
    skyboxShader.setMat4("projection", projection);

    float blendFactor = bindTextures();
    skyboxShader.setFloat("blendFactor", blendFactor);

    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_CLAMP);
}
