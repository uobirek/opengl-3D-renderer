#include "Lighting.h"

Lighting::Lighting() {
 
}

void Lighting::setLightingUniforms(Shader& lightingShader, const Camera& camera, int newTime, glm::vec3 spotlightPosition, glm::vec3 spotlightDirection) {
    lightingShader.setVec3("viewPos", camera.Position);
    lightingShader.setFloat("material.shininess", 32.0f);
    this->skyboxTime = newTime;
    updateDirectionalLight(lightingShader);
    // Reflector spotlight
    lightingShader.setVec3("spotLights[0].position", spotlightPosition);
    lightingShader.setVec3("spotLights[0].direction", spotlightDirection);
    lightingShader.setVec3("spotLights[0].ambient", 0.2f, 0.2f, 0.8f);
    lightingShader.setVec3("spotLights[0].diffuse", 0.3f, 0.3f, 0.8f);
    lightingShader.setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLights[0].constant", 1.0f);
    lightingShader.setFloat("spotLights[0].linear", 0.09f);
    lightingShader.setFloat("spotLights[0].quadratic", 0.032f);
    lightingShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
    lightingShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));

    // Camera spotlight
    lightingShader.setVec3("spotLights[1].position", camera.Position);
    lightingShader.setVec3("spotLights[1].direction", camera.Front);
    lightingShader.setVec3("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLights[1].diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader.setVec3("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLights[1].constant", 1.0f);
    lightingShader.setFloat("spotLights[1].linear", 0.09f);
    lightingShader.setFloat("spotLights[1].quadratic", 0.032f);
    lightingShader.setFloat("spotLights[1].cutOff", glm::cos(glm::radians(12.5f)));
    lightingShader.setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(15.0f)));

    // Point lights (same as before)
    for (size_t i = 0; i < pointLightPositions.size(); ++i) {
        std::string index = std::to_string(i);
        lightingShader.setVec3("pointLights[" + index + "].position", pointLightPositions[i]);
        lightingShader.setVec3("pointLights[" + index + "].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[" + index + "].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[" + index + "].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[" + index + "].constant", 1.0f);
        lightingShader.setFloat("pointLights[" + index + "].linear", 0.09f);
        lightingShader.setFloat("pointLights[" + index + "].quadratic", 0.032f);
    }

}



void Lighting::updateDirectionalLight(Shader& lightingShader) {

    glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f); // Default direction
    glm::vec3 ambient, diffuse, specular;

    if (skyboxTime >= 0 && skyboxTime < 5000) {
        // Nighttime
        ambient = glm::vec3(0.01f, 0.01f, 0.02f);   // Dim blueish ambient light
        diffuse = glm::vec3(0.1f, 0.1f, 0.15f);     // Soft moonlight
        specular = glm::vec3(0.2f, 0.2f, 0.3f);     // Gentle specular for moonlight
    }
    else if (skyboxTime >= 5000 && skyboxTime < 6000) {
        // Transition: Night to Day
        float factor = (float)(skyboxTime - 5000) / (6000 - 5000);
        ambient = glm::mix(glm::vec3(0.01f, 0.01f, 0.02f), glm::vec3(0.03f, 0.03f, 0.04f), factor);
        diffuse = glm::mix(glm::vec3(0.1f, 0.1f, 0.15f), glm::vec3(1.0f, 0.95f, 0.8f), factor);
        specular = glm::mix(glm::vec3(0.2f, 0.2f, 0.3f), glm::vec3(1.0f, 0.9f, 0.7f), factor);
    }
    else if (skyboxTime >= 6000 && skyboxTime < 21000) {
        // Daytime
        ambient = glm::vec3(0.03f, 0.03f, 0.04f); // Slight ambient to simulate global illumination
        diffuse = glm::vec3(1.0f, 0.95f, 0.8f);   // Bright sunlight
        specular = glm::vec3(1.0f, 0.9f, 0.7f);   // Strong highlights for shiny surfaces
    }
    else if (skyboxTime >= 21000 && skyboxTime < 22000) {
        // Transition: Day to  Night
        float factor = (float)(skyboxTime - 21000) / (22000 - 21000);
        ambient = glm::mix(glm::vec3(0.03f, 0.03f, 0.04f), glm::vec3(0.01f, 0.01f, 0.02f), factor);
        diffuse = glm::mix(glm::vec3(1.0f, 0.95f, 0.8f), glm::vec3(0.1f, 0.1f, 0.15f), factor);
        specular = glm::mix(glm::vec3(1.0f, 0.9f, 0.7f), glm::vec3(0.2f, 0.2f, 0.3f), factor);
    }
    else {
        // Nighttime
        ambient = glm::vec3(0.01f, 0.01f, 0.02f);
        diffuse = glm::vec3(0.1f, 0.1f, 0.15f);
        specular = glm::vec3(0.2f, 0.2f, 0.3f);
    }

    // Set the light properties in the shader
    lightingShader.setVec3("dirLight.direction", direction.x, direction.y, direction.z);
    lightingShader.setVec3("dirLight.ambient", ambient.x, ambient.y, ambient.z);
    lightingShader.setVec3("dirLight.diffuse", diffuse.x, diffuse.y, diffuse.z);
    lightingShader.setVec3("dirLight.specular", specular.x, specular.y, specular.z);
}

void Lighting::setPointLightPositions(const std::vector<glm::vec3>& positions) {
    pointLightPositions = positions;
}

void Lighting::setupLightCubeVAO() {
    // Vertices for the light cube (same as the ones for the regular cube)
    float vertices[] = {
           -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
           -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

           -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
           -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
           -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

           -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
           -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
           -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
           -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

           -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
           -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

           -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
           -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
           -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    // Set up the light cube VAO and VBO
    unsigned int VBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Unbind the VAO after setting up
}

void Lighting::drawLightCubes(Shader& lightCubeShader, const glm::mat4& view, const glm::mat4& projection) {
    lightCubeShader.use();
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("view", view);

    glBindVertexArray(lightCubeVAO);
    for (size_t i = 0; i < pointLightPositions.size(); i++) {
        std::cout << pointLightPositions.size() << std::endl;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[i]);
        model = glm::scale(model, glm::vec3(0.2f));  // Smaller cubes to represent point lights
        lightCubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);  // Drawing the cube with 36 vertices
    }


}