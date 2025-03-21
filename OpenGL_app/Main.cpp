﻿#define STB_IMAGE_IMPLEMENTATION
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


void updateDeltaTime();

void prepareFrame();



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

glm::vec3 manualOffset = glm::vec3(0.0f, 0.0f, 0.0f); // Default manual adjustment

bool isFollowingSphere;
bool isLookingAtSphere;

// Default values
float specularIntensity = 0.9f;  // Ks (default = 0.5)
float shininessValue = 60.0f;    // Shininess (default = 32)

bool blinn = false;
bool blinnKeyPressed = false;

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
//	Shader lightingShader("shader.vs", "shader.fs");
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
	//lightingShader.use();
	//lightingShader.setInt("material.diffuse", 0);
	//lightingShader.setInt("material.specular", 1);

	stbi_set_flip_vertically_on_load(false);

	Skybox skybox(skyboxShader);
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


	float planeVertices[] = {
		// positions            // normals         // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};
	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);


	while (!glfwWindowShouldClose(window))
	{

		

		updateDeltaTime();
		processInput(window);

		prepareFrame();

		// ----------- GEOMETRY PASS ----------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		shaderGeometryPass.use();
		shaderGeometryPass.setMat4("projection", projection);
		shaderGeometryPass.setMat4("view", view);

		shaderGeometryPass.setBool("useTexture", true);
		float time = static_cast<float>(glfwGetTime()); 
		glm::mat4 model = glm::mat4(1.0f);

		
		float xPos = 15.0f + abs(sin(time)) * 5.0f; 
		float zPos = 15.0f + abs(cos(time)) * 5.0f; 
		model = glm::translate(model, glm::vec3(xPos, 1.0f, zPos));
		float angle = time; 
		model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f)); 

		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		shaderGeometryPass.setMat4("model", model);

		ourModel.Draw(shaderGeometryPass);




		shaderGeometryPass.setVec3("fixedColor", 1.0f, 0.7f, 0.1f);  

		shaderGeometryPass.setBool("useTexture", false);

		// Render the sphere model
	// Static sphere (no movement)
		glm::mat4 staticSphereModel = glm::mat4(1.0f);
		staticSphereModel = glm::translate(staticSphereModel, glm::vec3(0.0f, 0.0f, 5.0f));
		staticSphereModel = glm::scale(staticSphereModel, glm::vec3(0.5f));

		staticSphere.updateModelMatrix(staticSphereModel);
		staticSphere.setShaderAttributes(shaderGeometryPass);
		staticSphere.render();


		shaderGeometryPass.setVec3("fixedColor", 0.5f, 0.7f, 0.1f);  // Green ambient

		// Sphere movement (oscillating along X-axis)
		glm::mat4 movingSphereModel = glm::mat4(1.0f);
		movingSphereModel = glm::translate(movingSphereModel, glm::vec3(5.0f, 0.0f, 0.0f));
		movingSphereModel = glm::scale(movingSphereModel, glm::vec3(0.5f));

		float sphereMovementRange = 2.0f;  // How far the sphere moves
		float sphereSpeed = 1.0f;  // Speed of movement

		glm::vec3 centerPosition = glm::vec3(5.0f, 0.0f, 0.0f);
		glm::vec3 animatedOffset = glm::vec3(
			sin(glfwGetTime() * sphereSpeed) * sphereMovementRange,  // Moves back & forth along X-axis
			0.0f,
			0.0f
		);

		// Apply translation for animation
		movingSphereModel = glm::translate(movingSphereModel, animatedOffset);
		movingSphere.updateModelMatrix(movingSphereModel);
		movingSphere.setShaderAttributes(shaderGeometryPass);
		movingSphere.render();
		glm::vec3 spherePosition = centerPosition + animatedOffset;
		// THIRD-PERSON CAMERA (Following Behind the Sphere)
		if(isFollowingSphere) {
			float cameraDistance = 0.0f;  
			float cameraHeight = 2.0f;    

			glm::vec3 cameraOffset = glm::vec3(0.0f, cameraHeight, -cameraDistance);
			glm::vec3 cameraPosition = spherePosition + cameraOffset;

			camera.Position = cameraPosition;

			glm::vec3 fixedCubePosition = glm::vec3(10.0f, 0.0f, 0.0f);
			camera.Front = glm::normalize(fixedCubePosition - cameraPosition);

			camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		else if (isLookingAtSphere) {
			glm::vec3 fixedCameraPosition = glm::vec3(0.0f, 5.0f, -6.0f); // Camera in a fixed spot
			camera.Position = fixedCameraPosition;
			camera.Front = glm::normalize(spherePosition - fixedCameraPosition);
		}



		// Cube transformation
		glm::mat4 cubeModel = glm::mat4(1.0f);
		cubeModel = glm::translate(cubeModel, glm::vec3(10.0f, 0.0f, 0.0f));
		cubeModel = glm::scale(cubeModel, glm::vec3(3.0f));                  

		// Update cube's model matrix and render it
		cube.updateModelMatrix(cubeModel);
		cube.setShaderAttributes(shaderGeometryPass);
		cube.render();

		spherePosition = centerPosition + animatedOffset;
		glm::vec3 normalDirection = glm::normalize(animatedOffset); 
		float sphereRadius = 0.5f; 

		glm::vec3 spotlightPosition = spherePosition + normalDirection * sphereRadius; 

		glm::vec3 cubePosition = glm::vec3(10.0f, 0.0f, 0.0f);

		glm::vec3 spotlightDirection = glm::normalize((cubePosition - spotlightPosition) + manualOffset);

		shaderGeometryPass.setVec3("fixedColor", 0.5f, 0.1f, 0.7f);  

		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);




		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// --------------lIGHTING PASS -------------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderLightingPass.use();
		shaderLightingPass.setInt("gPosition", 0);
		shaderLightingPass.setInt("gNormal", 1);
		shaderLightingPass.setInt("gAlbedoSpec", 2);

		shaderLightingPass.setInt("blinn", blinn);

		std::cout << (blinn ? "Blinn-Phong" : "Phong") << std::endl;
		
		shaderLightingPass.setFloat("Ks", specularIntensity);
		shaderLightingPass.setFloat("shininess", shininessValue);


		std::vector<glm::vec3> pointLightPositions;
	//	pointLightPositions.push_back(glm::vec3(2.0f, 1.0f, -2.0f)); // Point Light 1
		pointLightPositions.push_back(glm::vec3(0.0f,0.0f, 0.0f)); // Point Light 2
		pointLightPositions.push_back(glm::vec3(0.0f, 1.0f, 3.0f)); // Point Light 2

		pointLightPositions.push_back(glm::vec3(2.0f, 4.0f, 0.0f)); // Point Light 2
		pointLightPositions.push_back(glm::vec3(3.0f, 0.0f, 1.0f)); // Point Light 2

		lighting.setPointLightPositions(pointLightPositions);
		lighting.setLightingUniforms(shaderLightingPass, camera, skyboxTime, spotlightPosition, spotlightDirection);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		for (unsigned int i = 0; i < lightPositions.size(); i++) {
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
			shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Linear", 0.7f);
			shaderLightingPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", 1.8f);
		}

		shaderLightingPass.setVec3("viewPos", camera.Position);

		renderQuad();
		// ------------- POST PROCESSING -----------

		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		lightCubeShader.use();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		lighting.drawLightCubes(lightCubeShader, view, projection);


		// ------------- SKYBOX -------------

		skybox.render(camera.GetViewMatrix(), projection, skyboxTime, deltaTime);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

void updateDeltaTime() {
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	skyboxTime += deltaTime * 1000;
	skyboxTime = fmod(skyboxTime, 24000.0f); // Keep skybox time between 0 and 24000
}

void prepareFrame() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
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

float manualSpeed = 0.02f; // Sensitivity for manual spotlight adjustments

void processInput(GLFWwindow* window)
{
	// Close the window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Toggle between following the sphere and manual camera movement
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		// Toggle camera mode
		isFollowingSphere = true;
		isLookingAtSphere = false;

		std::cout << isFollowingSphere << "   " << isLookingAtSphere << std::endl;


	}
	// Toggle between following the sphere and manual camera movement
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		// Toggle camera mode
		isLookingAtSphere = true;
		isFollowingSphere = false;
		std::cout << isFollowingSphere << "   " << isLookingAtSphere << std::endl;


	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		// Toggle camera mode
		isLookingAtSphere = false;
		isFollowingSphere = false;
		std::cout << isFollowingSphere << "   " << isLookingAtSphere << std::endl;

	}

	// If the camera is in manual mode, allow movement
	if (!isFollowingSphere)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltaTime);

		// Spotlight manual direction control
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			manualOffset.y += manualSpeed; // Move spotlight direction up
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			manualOffset.y -= manualSpeed; // Move spotlight direction down
	}

	// Spotlight reset
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		manualOffset = glm::vec3(0.0f, 0.0f, 0.0f); // Reset manual adjustment

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
		shininessValue = std::min(shininessValue + 1.0f, 128.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		shininessValue = std::max(shininessValue - 1.0f, 8.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		specularIntensity = std::min(specularIntensity + 0.05f, 1.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		specularIntensity = std::max(specularIntensity - 0.05f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
	{
		blinn = !blinn;
		blinnKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
	{
		blinnKeyPressed = false;
	}
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
	if (!isFollowingSphere && !isLookingAtSphere) {
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
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!isFollowingSphere) {
		camera.ProcessMouseScroll(static_cast<float>(yoffset));
	}
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


