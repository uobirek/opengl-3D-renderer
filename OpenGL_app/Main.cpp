#define STB_IMAGE_IMPLEMENTATION
// External libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Utilities
#include "stb_image.h"

// Project-specific headers
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "skybox.h"
#include "lighting.h"
#include "sphere.h"
#include "cube.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(const char* path);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderQuad();




// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int skyboxTime = 0;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    stbi_set_flip_vertically_on_load(true);



    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader lightingShader("shader.vs", "shader.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    Shader skyboxShader("skybox_shader.vs", "skybox_shader.fs");


    Shader shaderGeometryPass("g_buffer.vs", "g_buffer.fs");
    Shader shaderLightingPass("deferred.vs", "deferred.fs");


    // load models
    // -----------
    Model ourModel("backpack/backpack.obj");
    //Lighting lighting;
    //lighting.setPointLightPositions({ glm::vec3(0.7f, 0.2f, 2.0f) });  // Set the point light positions


    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture("backpack/diffuse.jpg");
    unsigned int specularMap = loadTexture("backpack/specular.jpg");

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    stbi_set_flip_vertically_on_load(false);

    Skybox skybox( skyboxShader);
    Sphere staticSphere;   // Static sphere (blue)
    Sphere movingSphere;
    Cube cube;

    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
    objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));
    // configure g-buffer framebuffer
    // ------------------------------
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // lighting info
    // -------------
    const unsigned int NR_LIGHTS = 2; // Only two lights now
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    // Define positions for two lights
    lightPositions.push_back(glm::vec3(2.0f, 1.0f, -2.0f)); // Light 1 position
    lightPositions.push_back(glm::vec3(-1.0f, -1.0f, 2.0f)); // Light 2 position

    // Define colors for two lights
    lightColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f)); // Warm light
    lightColors.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // Cool light


    // shader configuration
    // --------------------
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedoSpec", 2);

    Cube lightcubespecial;


    Lighting lighting;



    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        skyboxTime += deltaTime * 1000;
        skyboxTime %= 24000;

        processInput(window);

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);


        // LIGHT
      //  lightingShader.use();
       // lightingShader.setVec3("viewPos", camera.Position);
       // lightingShader.setFloat("material.shininess", 32.0f);




        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
     //   lightingShader.setMat4("projection", projection);
      //  lightingShader.setMat4("view", view);

        shaderGeometryPass.use();
        shaderGeometryPass.setMat4("projection", projection);
        shaderGeometryPass.setMat4("view", view);


        // BACKPACK
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the backpack at the origin
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // Scale it as needed
      //  lightingShader.setMat4("model", model);
       // lightingShader.setBool("material.useTextures", true);

        shaderGeometryPass.setBool("material.useTextures", true);


        shaderGeometryPass.setMat4("model", model);
        ourModel.Draw(shaderGeometryPass);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Update lighting uniforms


        // Lighting info
        const unsigned int NR_POINT_LIGHTS = 2; // Two point lights
        const unsigned int NR_SPOT_LIGHTS = 1; // One spotlight

        // Point light positions and colors
        std::vector<glm::vec3> pointLightPositions;
        std::vector<glm::vec3> pointLightColors;

        pointLightPositions.push_back(glm::vec3(2.0f, 1.0f, -2.0f)); // Point Light 1
        pointLightPositions.push_back(glm::vec3(-1.0f, -1.0f, 2.0f)); // Point Light 2

        pointLightColors.push_back(glm::vec3(1.0f, 1.0f, 1.0f)); // Point Light 1 color
        pointLightColors.push_back(glm::vec3(0.0f, 0.0f, 1.0f)); // Point Light 2 color


        // Shader configuration
        shaderLightingPass.use();
        shaderLightingPass.setInt("gPosition", 0);
        shaderLightingPass.setInt("gNormal", 1);
        shaderLightingPass.setInt("gAlbedoSpec", 2);

        lighting.setPointLightPositions(pointLightPositions);
        lighting.setLightingUniforms(shaderLightingPass, camera, skyboxTime, glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(5.0f, 0.0f, 3.0f));


        /// LIGHT PASS
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        // send light relevant uniforms
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
            shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
            // update attenuation parameters and calculate radius
            const float linear = 0.7f;
            const float quadratic = 1.8f;
            shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
            shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
        }
        shaderLightingPass.setVec3("viewPos", camera.Position);

     renderQuad();




        // COPY CONTENT OF GEOMETRY'S DEPTH
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);



        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        lighting.drawLightCubes(lightCubeShader, view, projection);
        // finally render quad



        //SKY BOX
        skybox.render(camera.GetViewMatrix(), projection, skyboxTime, deltaTime);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        skyboxTime += deltaTime * 1000;
        skyboxTime %= 24000;

        processInput(window);

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);


        // LIGHT
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);



        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);


        // BACKPACK
        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the backpack at the origin
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // Scale it as needed
        lightingShader.setMat4("model", model);
        lightingShader.setBool("material.useTextures", true);
        ourModel.Draw(lightingShader);

        // SPHERES
        // Shader settings (for spheres)
        lightingShader.setVec3("material.ambientColor", 0.2f, 0.2f, 0.8f);  // Blue ambient
        lightingShader.setVec3("material.diffuseColor", 0.3f, 0.3f, 0.8f);  // Blue diffuse
        lightingShader.setVec3("material.specularColor", 1.0f, 1.0f, 1.0f); // White specular
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setBool("material.useTextures", false);

        // Static sphere (no movement)
        glm::mat4 staticSphereModel = glm::mat4(1.0f);
        staticSphereModel = glm::translate(staticSphereModel, glm::vec3(0.0f, 0.0f, 5.0f));
        staticSphereModel = glm::scale(staticSphereModel, glm::vec3(0.5f));

        staticSphere.updateModelMatrix(staticSphereModel);
        staticSphere.setShaderAttributes(lightingShader);
        staticSphere.render();

        // Moving sphere (animated over time)
        glm::mat4 movingSphereModel = glm::mat4(1.0f);
        movingSphereModel = glm::translate(movingSphereModel, glm::vec3(5.0f, 0.0f, 0.0f));
        movingSphereModel = glm::scale(movingSphereModel, glm::vec3(0.5f));
        // Oscillating the sphere position along the X-axis (between 5.0f and 7.0f)
        float sphereMovementRange = 2.0f;  // Controls how far the sphere moves
        float sphereSpeed = 1.0f;  // Controls the speed of the movement

        glm::vec3 centerPosition = glm::vec3(5.0f, 0.0f, 0.0f); // Sphere's original center
        glm::vec3 animatedOffset = glm::vec3(
            sin(glfwGetTime() * sphereSpeed) * sphereMovementRange,  // Oscillates back and forth between -sphereMovementRange and +sphereMovementRange
            0.0f,  // No vertical movement
            0.0f   // No movement along the Z-axis
        );

        movingSphereModel = glm::translate(movingSphereModel, animatedOffset);
        movingSphere.updateModelMatrix(movingSphereModel);
        movingSphere.setShaderAttributes(lightingShader);
        movingSphere.render();



        // Cube Shader settings (similar to sphere but with green tones)
        lightingShader.setVec3("material.ambientColor", 0.1f, 0.7f, 0.1f);  // Green ambient
        lightingShader.setVec3("material.diffuseColor", 0.3f, 0.7f, 0.3f);  // Green diffuse
        lightingShader.setVec3("material.specularColor", 1.0f, 1.0f, 1.0f); // White specular
        lightingShader.setFloat("material.shininess", 50.0f);                // High shininess
        lightingShader.setBool("material.useTextures", false);

        // Cube transformation
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(10.0f, 0.0f, 0.0f)); // Translate to position
        cubeModel = glm::scale(cubeModel, glm::vec3(2.0f));                  // Scale uniformly

        // Update cube's model matrix and render it
        cube.updateModelMatrix(cubeModel);
        cube.setShaderAttributes(lightingShader);
        cube.render();



        // LIGHT PART - Place Spotlight on Surface of Moving Sphere
        glm::vec3 spherePosition = centerPosition + animatedOffset; // Final center position
        glm::vec3 normalDirection = glm::normalize(animatedOffset);  // Normal from center to surface
        float sphereRadius = 0.5f;  // Scale factor used earlier

        glm::vec3 spotlightPosition = spherePosition + normalDirection * sphereRadius; // Move light to surface

        glm::vec3 cubePosition = glm::vec3(8.0f, 0.0f, 0.0f);
        glm::vec3 spotlightDirection = glm::normalize(cubePosition - spotlightPosition);

        lighting.setLightingUniforms(lightingShader, camera, skyboxTime, spotlightPosition, spotlightDirection);

        // LIGHT CUBE - TODO: not working
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        lighting.drawLightCubes(lightCubeShader, view, projection);

        //SKY BOX
        skybox.render(camera.GetViewMatrix(), projection, skyboxTime, deltaTime);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}
unsigned int quadVAO = 0, quadVBO;

void renderQuad()
{
    if (quadVAO == 0)
    {
        // Define vertices for a screen-filling quad
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };

        // Create VAO and VBO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    // Render the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


