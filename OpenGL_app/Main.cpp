

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
#include "../sphere/sphere/src/Sphere.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int loadTexture(const char* path);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(vector<std::string> faces);
float bindTextures( unsigned int texture, unsigned int nightTexture);
void updateDirectionalLight(Shader& lightingShader);


void renderSphere();

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

float ROTATE_SPEED = 1;
float rotation = 0;

int skybox_time = 0;

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

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader lightingShader("shader.vs", "shader.fs");
    Shader lightCubeShader("light_cube.vs", "light_cube.fs");
    Shader skyboxShader("skybox_shader.vs", "skybox_shader.fs");

    // load models
    // -----------
    Model ourModel("backpack/backpack.obj");

    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    glm::vec3 pointLightPositions[] = {
       glm::vec3(0.7f,  0.2f,  2.0f),

    };

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int VBO, lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture("backpack/diffuse.jpg");
    unsigned int specularMap = loadTexture("backpack/specular.jpg");

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    stbi_set_flip_vertically_on_load(false);

    vector<std::string> day_faces
    {
        "skybox/right.jpg",
            "skybox/left.jpg",
            "skybox/top.jpg",
            "skybox/bottom.jpg",
            "skybox/front.jpg",
            "skybox/back.jpg"
    };
    vector<std::string> night_faces
    {
        "skybox/nightRight.jpg",
            "skybox/nightLeft.jpg",
            "skybox/nightTop.jpg",
            "skybox/nightBottom.jpg",
            "skybox/nightFront.jpg",
            "skybox/nightBack.jpg"
    };
    unsigned int day_cubemapTexture = loadCubemap(day_faces);
    unsigned int night_cubemapTexture = loadCubemap(night_faces);




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


    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    skyboxShader.use();
    glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
    glBindTexture(GL_TEXTURE_CUBE_MAP, day_cubemapTexture); // Bind day texture
    skyboxShader.setInt("skybox", 0); // Set sampler to use texture unit 0

    glActiveTexture(GL_TEXTURE1); // Activate texture unit 1
    glBindTexture(GL_TEXTURE_CUBE_MAP, night_cubemapTexture); // Bind night texture
    skyboxShader.setInt("skybox_night", 1); // Set sampler to use texture unit 1
    skyboxShader.setInt("blendFactor", 2);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        lightingShader.use();

        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
        updateDirectionalLight(lightingShader);
        // point light 1
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.09f);
        lightingShader.setFloat("pointLights[0].quadratic", 0.032f);

        // spotLight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


        glEnable(GL_DEPTH_TEST);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        ////// world transformation
        //glm::mat4 model = glm::mat4(1.0f);
        //lightingShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);


        // Render the backpack
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the backpack at the origin
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // Scale it as needed
        lightingShader.setMat4("model", model);
        lightingShader.setBool("material.useTextures", true);
        ourModel.Draw(lightingShader);

        // Render the sphere
        glm::mat4 sphereModel = glm::mat4(1.0f);

        sphereModel = glm::translate(sphereModel, glm::vec3(0.0f, 0.0f, 5.0f)); // Move the sphere to the right
        sphereModel = glm::scale(sphereModel, glm::vec3(0.5f, 0.5f, 0.5f));     // Scale it to half its size
        lightingShader.setVec3("material.ambientColor", 0.2f, 0.2f, 0.8f);  // Example blueish ambient
        lightingShader.setVec3("material.diffuseColor", 0.3f, 0.3f, 0.8f);  // Example blueish diffuse
        lightingShader.setVec3("material.specularColor", 1.0f, 1.0f, 1.0f); // White specular
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setBool("material.useTextures", false);
        lightingShader.setMat4("model", sphereModel);

        renderSphere();

        // Render the sphere
        glm::mat4 movingSphereModel = glm::mat4(1.0f);

        movingSphereModel = glm::translate(movingSphereModel, glm::vec3(5.0f, 0.0f, 0.0f)); // Move the sphere to the right
        movingSphereModel = glm::scale(movingSphereModel, glm::vec3(0.5f, 0.5f, 0.5f));     // Scale it to half its size
        lightingShader.setVec3("material.ambientColor", 0.2f, 0.2f, 0.8f);  // Example blueish ambient
        lightingShader.setVec3("material.diffuseColor", 0.3f, 0.3f, 0.8f);  // Example blueish diffuse
        lightingShader.setVec3("material.specularColor", 1.0f, 1.0f, 1.0f); // White specular
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setBool("material.useTextures", false);


        movingSphereModel = glm::translate(movingSphereModel, glm::vec3(1.0f + sin(glfwGetTime()) * 2.0f, sin(glfwGetTime() / 2.0f) * 1.0f, 0.0f));
        /*float angle = 20.0f;

            angle = glfwGetTime() * 25.0f;
        movingSphereModel = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));*/
        lightingShader.setMat4("model", movingSphereModel);

        renderSphere();


        // also draw the lamp object(s)
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }


        // draw skybox as last
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_CLAMP);// change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        rotation += ROTATE_SPEED * glfwGetTime() * 0.001;
        view = glm::rotate(view, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        float blendFactor = bindTextures(day_cubemapTexture, night_cubemapTexture);
        skyboxShader.setFloat("blendFactor", blendFactor);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
        glDisable(GL_DEPTH_CLAMP);

        //glm::mat4 model2 = glm::mat4(1.0f);
        //model2 = glm::translate(model, glm::vec3(0.2f, 0.2f, 0.2f)); // it's a bit too big for our scene, so scale it down
        //lightingShader.setMat4("model", model2);
        //secondModel.Draw(lightingShader);

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
void updateDirectionalLight(Shader& lightingShader) {
    glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f); // Default direction
    glm::vec3 ambient, diffuse, specular;

    if (skybox_time >= 0 && skybox_time < 5000) {
        // Nighttime
        ambient = glm::vec3(0.01f, 0.01f, 0.02f);   // Dim blueish ambient light
        diffuse = glm::vec3(0.1f, 0.1f, 0.15f);     // Soft moonlight
        specular = glm::vec3(0.2f, 0.2f, 0.3f);     // Gentle specular for moonlight
    }
    else if (skybox_time >= 5000 && skybox_time < 6000) {
        // Transition: Night to Day
        float factor = (float)(skybox_time - 5000) / (6000 - 5000);
        ambient = glm::mix(glm::vec3(0.01f, 0.01f, 0.02f), glm::vec3(0.03f, 0.03f, 0.04f), factor);
        diffuse = glm::mix(glm::vec3(0.1f, 0.1f, 0.15f), glm::vec3(1.0f, 0.95f, 0.8f), factor);
        specular = glm::mix(glm::vec3(0.2f, 0.2f, 0.3f), glm::vec3(1.0f, 0.9f, 0.7f), factor);
    }
    else if (skybox_time >= 6000 && skybox_time < 21000) {
        // Daytime
        ambient = glm::vec3(0.03f, 0.03f, 0.04f); // Slight ambient to simulate global illumination
        diffuse = glm::vec3(1.0f, 0.95f, 0.8f);   // Bright sunlight
        specular = glm::vec3(1.0f, 0.9f, 0.7f);   // Strong highlights for shiny surfaces
    }
    else if (skybox_time >= 21000 && skybox_time < 22000) {
        // Transition: Day to  Night
        float factor = (float)(skybox_time - 21000) / (22000 - 21000);
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


    float bindTextures(unsigned int texture, unsigned int nightTexture) {
        skybox_time += deltaTime * 1000; // Simulate time progression
        skybox_time %= 24000; // Keep time within the range [0, 24000)

        int texture1;
        int texture2;
        float blendFactor = 0.0f; // Default blendFactor (no blending)

        if (skybox_time >= 0 && skybox_time < 5000) {
            // Full night
            texture1 = nightTexture;
            texture2 = nightTexture;
            blendFactor = 0.0f;
        }
        else if (skybox_time >= 5000 && skybox_time < 6000) {
            // Night to Day transition
            texture1 = nightTexture;
            texture2 = texture;
            blendFactor = (float)(skybox_time - 5000) / (6000 - 5000);
        }
        else if (skybox_time >= 6000 && skybox_time < 21000) {
            // Full day
            texture1 = texture;
            texture2 = texture;
            blendFactor = 0.0f;
        }
        else if (skybox_time >= 21000 && skybox_time < 22000) {
            // Day to Night transition
            texture1 = texture;
            texture2 = nightTexture;
            blendFactor = (float)(skybox_time - 21000) / (22000 - 21000);
        }
        else if (skybox_time >= 22000 && skybox_time < 24000) {
            // Full night
            texture1 = nightTexture;
            texture2 = nightTexture;
            blendFactor = 0.0f;
        }


        // Texture binding logic (comment out if unnecessary in this function)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture2);

        return blendFactor;
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


// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}
unsigned int loadCubemap(vector<std::string> faces)
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