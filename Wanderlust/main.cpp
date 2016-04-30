#include <iostream>
#include <memory>
#include <glew\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sstream>
#include "shader.h"
#include "Cube.h"
#include <Windows.h>
#include "Model.h"
#include "Mesh.h"
#include "Camera.h"

using namespace cgue;
using namespace cgue::scene;

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
void init(GLFWwindow* window);
void cleanup();
void draw();
void update(float time_delta);


std::unique_ptr<Camera> cam;

std::unique_ptr<Shader> shader;
std::unique_ptr<Shader> toonShader;

std::unique_ptr<Shader> sunShader;

std::unique_ptr<Model> player;
std::unique_ptr<Model> plattform;
std::unique_ptr<Model> sun;
std::unique_ptr<Model> path;

glm::mat4 persp;
glm::mat4 view;
glm::vec3 cameraPos;
//int i = 1;
float rad = 0.0f;
//std::unique_ptr<Cube> cube;


int main(){

	if (!glfwInit())
	{
		std::cerr << "Could not init glfw!" << std::endl;
		system("Pause");
		exit(EXIT_FAILURE);
	}

	const int width = 800;
	const int height = 600;
	

#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	auto window = glfwCreateWindow(width, height, "CGUE", nullptr, nullptr);

	if (!window)
	{
		std::cerr << ("ERROR : Could not create window!") << std::endl;
		glfwTerminate();
		system("Pause");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << ("ERROR : Could not load glew!") << std::endl;
		glfwTerminate();
		system("Pause");
		exit(EXIT_FAILURE);
	}


#if _DEBUG
	// Query the OpenGL function to register your callback function.
	PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
	PFNGLDEBUGMESSAGECALLBACKARBPROC _glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
	PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)wglGetProcAddress("glDebugMessageCallbackAMD");

	// Register your callback function.
	if (_glDebugMessageCallback != NULL) {
		_glDebugMessageCallback(DebugCallback, NULL);
	}
	else if (_glDebugMessageCallbackARB != NULL) {
		_glDebugMessageCallbackARB(DebugCallback, NULL);
	}
	

	// Enable synchronous callback. This ensures that your callback function is called
	// right after an error has occurred. This capability is not defined in the AMD
	// version.
	if ((_glDebugMessageCallback != NULL) || (_glDebugMessageCallbackARB != NULL)) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
#endif



	init(window);

	glClearColor(0.35f, 0.36f, 0.43f, 0.3f);
	glViewport(0, 0, width, height);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	auto time = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{

		auto time_new = glfwGetTime();
		auto time_delta = (float)(time_new - time);
		time = time_new;

		/*std::cout << "frametime:" << time_delta * 1000 << "ms"
			<< " =~" << 1.0 / time_delta << "fps" << std::endl;*/

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		rad += (glm::pi<float>() / 180.0f) *  20 * time_delta;
		plattform->position = glm::vec3(sin(rad) * 10.0f, 0, 0);
		plattform->update();
		draw();
		
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, true);
		}
		
		if (glfwGetKey(window, GLFW_KEY_F1))
		{
			glTranslatef(0.0f, 0.0f, 0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_A)){
			player->position.x -= 10 * time_delta;
			player->update();

			view = cam->update(glm::vec3(cam-> eyeX -= 10 * time_delta, cam->eyeY, cam ->eyeZ), player->position);
			GLint model_view = glGetUniformLocation(shader->programHandle, "view");
			glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
		}
		else if (glfwGetKey(window, GLFW_KEY_D))
		{
			player->position.x += 10* time_delta;
			player->update();

			view = cam->update(glm::vec3(cam->eyeX += 10 * time_delta, cam->eyeY, cam->eyeZ), player->position);
			GLint model_view = glGetUniformLocation(shader->programHandle, "view");
			glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
		}

		if (glfwGetKey(window, GLFW_KEY_W))
		{
			player->position.z -= 10 * time_delta;
			player->update();

			view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ -= 10 * time_delta), player->position);
			GLint model_view = glGetUniformLocation(shader->programHandle, "view");
			glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
		}
		else if (glfwGetKey(window, GLFW_KEY_S))
		{
			player->position.z += 10 * time_delta;
			player->update();

			view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ += 10 * time_delta), player->position);
			GLint model_view = glGetUniformLocation(shader->programHandle, "view");
			glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
		}
	}

	cleanup();

	glfwTerminate();

	return EXIT_SUCCESS;
}

void init(GLFWwindow* window)
{

	shader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/basic.frag");
	toonShader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/toon.frag");
	sunShader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/sun.frag");
	//cube = std::make_unique<Cube>(glm::mat4(1.0f), shader.get());
	cam = std::make_unique<Camera>(0.0f, 2.0f, 8.0f);
	player = std::make_unique<Model>("../Models/player.dae");
	plattform = std::make_unique<Model>("../Models/plattform.dae");
	sun = std::make_unique<Model>("../Models/sun.dae");
	path = std::make_unique<Model>("../Models/path_test.dae");

	toonShader->useShader();
	glm::mat4 projection;

	view = cam->setUp(player->position);

	projection = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);
	player->viewMatrix = view;
	path->viewMatrix = view;
	path->position = glm::vec3(0, -2.0f, 0);
	path->update();
	player->position = glm::vec3(0, 0, 0);
	std::cout << player->angle << std::endl;
	player->update();
	std::cout << player->angle << std::endl;
	plattform->position = glm::vec3(0, 3.0f, 0);
	plattform->viewMatrix = view;
	plattform->update();
	

	GLint modelView = glGetUniformLocation(toonShader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));

	GLint modelProjection = glGetUniformLocation(toonShader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));
	
	//Setting MaterialProperties

	/*GLint matAmbientPos = glGetUniformLocation(toonShader->programHandle, "mat.ambient");
	GLint matDiffusePos = glGetUniformLocation(toonShader->programHandle, "mat.diffuse");
	GLint matSpecularPos = glGetUniformLocation(toonShader->programHandle, "mat.specular");
	GLint matShinePos = glGetUniformLocation(toonShader->programHandle, "mat.shininess");

	glUniform3f(matAmbientPos, 1.0f, 0.5f, 0.3f);
	glUniform3f(matDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(matSpecularPos, 1.0f, 1.0f, 1.0f);
	glUniform1f(matShinePos, 10.0f);*/
	

	//Setting LightProperties

	GLint lightAmbientPos = glGetUniformLocation(toonShader->programHandle, "light.ambient");
	GLint lightDiffusePos = glGetUniformLocation(toonShader->programHandle, "light.diffuse");
	//GLint lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
	GLint lightDirectionPos = glGetUniformLocation(toonShader->programHandle, "light.direction");

	glUniform3f(lightDirectionPos, -1.0f, 1.0f, -1.0f);
	glUniform3f(lightAmbientPos, 0.2f, 0.2f, 0.2f);
	glUniform3f(lightDiffusePos, 1.0f, 1.0f, 1.0f);
	//glUniform3f(lightSpecularPos, 1.0f, 1.0f, 1.0f);
	
	sunShader->useShader();
	
	glm::vec3 lightPos(1.0f, 18.0f, -3.0f);
	sun->position = lightPos;
	sun->viewMatrix = view;
	sun->update();

	GLint modelLoc = glGetUniformLocation(sunShader->programHandle, "model");
	GLint viewLoc = glGetUniformLocation(sunShader->programHandle, "view");
	GLint projLoc = glGetUniformLocation(sunShader->programHandle, "projection");
	
	// Set matrices
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	
	//player->angle = glm::pi<float>();
	player->update();

}
void cleanup()
{

	player.reset(nullptr);
	shader.reset(nullptr);
	cam.reset(nullptr);
	sun.reset(nullptr);
	
}
void draw(){
			

	

	toonShader->useShader();
	player->viewMatrix = view;
	player->draw(shader.get());
	
	path->viewMatrix = view;
	path->draw(shader.get());

	plattform->viewMatrix = view;
	plattform->draw(shader.get());

	sunShader->useShader();
	sun->viewMatrix = view;
	sun->draw(sunShader.get());

}

void update(float time_delta)
{
	player->update();
}



static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
	switch (id) {
	case 131185: {
		// Warning: Video Memory as Source
	

		return;
	}
	default: {
		break;
	}
	}

	switch (id) {
	case 131218: {
		// Program/shader state performance warning:
		// Fragment Shader is going to be recompiled because the shader key based on GL state mismatches.

		return;
	}
	default: {
		break;
	}
	}
	
	std::string error = FormatDebugOutput(source, type, id, severity, message);
	std::cout << error << std::endl;
}

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
	std::stringstream stringStream;
	std::string sourceString;
	std::string typeString;
	std::string severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		typeString = "Portability";
		break;
	}
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		break;
	}
	}
	
	stringStream << "OpenGL Error: " << msg;
	stringStream << " [Source = " << sourceString;
	stringStream << ", Type = " << typeString;
	stringStream << ", Severity = " << severityString;
	stringStream << ", ID = " << id << "]";

	return stringStream.str();
}


