

#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "camera.h"
#include "stb_image.h"
#include "shader.h"
#include "model.h"
#include "skybox.h"
#include "lighting.h"
#include "sphere.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(const char* path);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);



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

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    float vertices[] = {
        // Positions           // Normals
        // Front face
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        // Back face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        // Left face
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,

        // Right face
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,

         // Bottom face
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
          0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,

         // Top face
         -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
          0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f
    };

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // Position

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // Normal

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader lightingShader("shader.vs", "shader.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    Shader skyboxShader("skybox_shader.vs", "skybox_shader.fs");


    Shader shaderGeometryPass("8.1.g_buffer.vs", "8.1.g_buffer.fs");
    Shader shaderLightingPass("8.1.deferred_shading.vs", "8.1.deferred_shading.fs");


    // load models
    // -----------
    Model ourModel("backpack/backpack.obj");
    Lighting lighting;
    lighting.setPointLightPositions({ glm::vec3(0.7f, 0.2f, 2.0f) });  // Set the point light positions


    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture("backpack/diffuse.jpg");
    unsigned int specularMap = loadTexture("backpack/specular.jpg");

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    stbi_set_flip_vertically_on_load(false);

    vector<std::string> dayFaces
    {
        "skybox/right.jpg",
            "skybox/left.jpg",
            "skybox/top.jpg",
            "skybox/bottom.jpg",
            "skybox/front.jpg",
            "skybox/back.jpg"
    };
    vector<std::string> nightFaces
    {
        "skybox/nightRight.jpg",
            "skybox/nightLeft.jpg",
            "skybox/nightTop.jpg",
            "skybox/nightBottom.jpg",
            "skybox/nightFront.jpg",
            "skybox/nightBack.jpg"
    };




    Skybox skybox(dayFaces, nightFaces, skyboxShader);
    Sphere staticSphere;   // Static sphere (blue)
    Sphere movingSphere;




    // render loop
    // -----------
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


        // CUBE
        lightingShader.setVec3("material.ambientColor", 0.1f, 0.7f, 0.1f);  
        lightingShader.setVec3("material.diffuseColor", 0.3f, 0.7f, 0.3f);  
        lightingShader.setVec3("material.specularColor", 1.0f, 1.0f, 1.0f); 
        lightingShader.setFloat("material.shininess", 50.0f);
        lightingShader.setBool("material.useTextures", false);

        glm::mat4 cubemodel = glm::mat4(1.0f);
        cubemodel = glm::translate(cubemodel, glm::vec3(10.0f, 0.0f, 0.0f));
        cubemodel = glm::scale(cubemodel, glm::vec3(2.0f));
        lightingShader.setMat4("model", cubemodel);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);



        // LIGHT PART - Place Spotlight on Surface of Moving Sphere
        glm::vec3 spherePosition = centerPosition + animatedOffset; // Final center position
        glm::vec3 normalDirection = glm::normalize(animatedOffset);  // Normal from center to surface
        float sphereRadius = 0.5f;  // Scale factor used earlier

        glm::vec3 spotlightPosition = spherePosition + normalDirection * sphereRadius; // Move light to surface

        // Compute direction vector from spotlight to cube
        glm::vec3 cubePosition = glm::vec3(8.0f, 0.0f, 0.0f);
        glm::vec3 spotlightDirection = glm::normalize(cubePosition - spotlightPosition);

        // Set lighting uniforms
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


