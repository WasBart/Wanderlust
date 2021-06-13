#include <iostream>
#include <memory>
#include <ctime>
#include <windows.h>
#include <glew\glew.h>
#include <GLFW\glfw3.h>
#include <PxPhysicsAPI.h> 
#include <extensions\PxExtensionsAPI.h>
#include <extensions\PxDefaultSimulationFilterShader.h>
#include <characterkinematic\PxCapsuleController.h>
#include <characterkinematic\PxControllerManager.h>
#include <characterkinematic\PxBoxController.h>




#ifdef _DEBUG
#pragma comment(lib, "PhysX3DEBUG_x86.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PhysX3CharacterKinematicDEBUG_x86.lib")
#pragma comment(lib, "PhysX3GpuDEBUG_x86.lib")

#else
#pragma comment(lib, "PhysX3_x86.lib")
#pragma comment(lib, "PhysX3Common_x86.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PhysX3CharacterKinematic_x86.lib")
#pragma comment(lib, "PhysX3Gpu_x86.lib")
#endif

#define GLM_SWIZZLE
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
#include "Frustum.h"
#include "ParticleSystem.h"
#include "Contour.h"
#include "TextRenderer.h"


using namespace cgue;
using namespace cgue::scene;

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

static physx::PxDefaultErrorCallback gDefaultErrorCallback;
static physx::PxDefaultAllocator gDefaultAllocatorCallback;
static physx::PxFoundation* gFoundation = NULL;
static physx::PxPhysics* gPhysicsSDK = NULL;
physx::PxScene* gScene = NULL;

physx::PxControllerManager* manager;
physx::PxCapsuleControllerDesc characterControllerDesc;
physx::PxController* characterController;

Frustum frustum;

//Function prototypes
void init(GLFWwindow* window);
void cleanup();
void draw();
void initPhysX();
void StepPhysX();
void update(float time_delta);
void mouseMovementPoll(GLFWwindow* window, double xpos, double ypos);
void keyboardInput(GLFWwindow*);
void renderShadowMap();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void RenderQuad();
void onShapeHit(const physx::PxControllerShapeHit& hit);
void print_youWin();
std::vector<std::vector<int>> calcPath();

boolean gourardOn = false;
float timer = 5.0f;

GLuint textureID;
std::unique_ptr<Shader> shader;
std::unique_ptr<Shader> shadowMapShader;
std::unique_ptr<Shader> skyBoxShader;
std::unique_ptr<Shader> gouradShader;

std::unique_ptr<Camera> cam;

std::vector<std::unique_ptr<Model>> models;
std::unique_ptr<Model> platforms[3][5];
bool marked[3][5];
std::unique_ptr<Model> player;
std::unique_ptr<Model> platform1_3;
std::unique_ptr<Model> platform0_3;
std::unique_ptr<Model> platform2_3;
std::unique_ptr<Model> platform0_2;
std::unique_ptr<Model> platform1_2;
std::unique_ptr<Model> platform2_2;
std::unique_ptr<Model> platform0_1;
std::unique_ptr<Model> platform1_1;
std::unique_ptr<Model> platform2_1;
std::unique_ptr<Model> platform0_0;
std::unique_ptr<Model> platform1_0;
std::unique_ptr<Model> platform2_0;
std::unique_ptr<Model> island;
std::unique_ptr<Model> plant;
std::unique_ptr<Model> platform2;
std::unique_ptr<Model> platform3;
std::unique_ptr<Model> goal;
std::unique_ptr<Model> island2;
std::unique_ptr<Model> island3;
std::unique_ptr<Model> island4;
std::unique_ptr<Model> skyBox;
std::unique_ptr<Model> platform4;
std::unique_ptr<Model> platform5;
std::unique_ptr<Model> platform6;
std::unique_ptr<Model> platform7;
std::unique_ptr<Model> platform8;
std::unique_ptr<Model> platform9;

std::unique_ptr<TextRenderer> text;

std::vector<GLuint> textures;
std::vector<std::vector<int>> paths;
std::vector<int> curPath;

//Border Array

std::vector<physx::PxRigidActor*> boundaries;
std::unique_ptr<ParticleSystem> parSys;
std::unique_ptr<Contour> contour;

glm::mat4 persp;
glm::mat4 view;
glm::mat4 projection;
glm::vec3 camPos;
glm::vec3 direction = glm::vec3(0.0,0.0,-1.0);
glm::mat4 lightProjection; 

const glm::vec3 camInitial = glm::vec3(0.0f, -0.3f, -1.0f);
glm::vec3 camDirection = camInitial;
const glm::vec4 clearColor = glm::vec4(0.10f, 0.10f, 0.10f, 1.0f);
float width;
float height;
float rad = 0.0f;
float time_delta;
float timeSim = 0.0f;
GLuint singleColorLoc;
float pathTimer = 0;


std::string message = "";
std::string textMessage = "";
float messageTimer = 0.0;
float winTimer = 0.0f;
float transp = 1.0f;

GLfloat near_plane = 0.1f, far_plane = 100.0f;
physx::PxReal myTimeStep = 1.0f / 60.0f;
physx::PxVec3 disp;
boolean gameOver = false;
boolean isJumping = false;
boolean frustumOn = true;
boolean frametimeOn = true;
boolean wireframeOn = false;
boolean texfilterOn = true;
int mipmapsOn = 0;
boolean transparencyOn = true;
bool accepted = false;

GLfloat lastX = 400, lastY = 300;
GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLuint depthMap;
glm::vec4 camUp = glm::vec4(0.0f,1.0f,0.0f,1.0f);

// Light source
glm::vec3 lightPos(-2.0f, 4.0f, 2.0f);


int main(int argc, char** argv){
	width = 800;
	height = 600;
	bool fullScreen = false;
	float refreshRate = 60;
	
	if (argc >= 5) {
	
		if ((std::stringstream(argv[1]) >> width).fail())
		{
			std::cerr << "ERROR: Could not parse first Element,try again" << std::endl;
			system("PAUSE");
			exit(EXIT_FAILURE);

		}
		lastX = width / 2.0f;

		if ((std::stringstream(argv[2]) >> height).fail())
		{
			std::cerr << "ERROR: Could not parse second Element,try again" << std::endl;
			system("PAUSE");
			exit(EXIT_FAILURE);

		}
		lastY = height / 2.0f;

		std::string full;

		if ((std::stringstream(argv[3]) >> full).fail())
		{
			std::cerr << "ERROR: Could not parse third Element,try again" << std::endl;
			system("PAUSE");
			exit(EXIT_FAILURE);

		}

		if (full.compare("full") == 0)
		{
			fullScreen = true;
		}

		if ((std::stringstream(argv[4]) >> refreshRate).fail())
		{
			std::cerr << "ERROR: Could not parse fourth Element,try again" << std::endl;
			system("PAUSE");
			exit(EXIT_FAILURE);

		}
	}
	if (!glfwInit())
	{
		std::cerr << "Could not init glfw!" << std::endl;
		system("Pause");
		exit(EXIT_FAILURE);
	}
	

#if _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window;

	if (fullScreen)
	{
		window = glfwCreateWindow(width, height, "CGUE", glfwGetPrimaryMonitor(), nullptr);
	}
	else
	{
		window = glfwCreateWindow(width, height, "CGUE", nullptr, nullptr);
	}

	if (!window)
	{
		std::cerr << ("ERROR : Could not create window!") << std::endl;
		glfwTerminate();
		system("Pause");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_REFRESH_RATE, refreshRate);

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
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
	initPhysX();

	glm::vec3 pos, eye;
	pos.x = characterController->getPosition().x;
	pos.y = characterController->getPosition().y;
	pos.z = characterController->getPosition().z;
	eye = pos - camDirection * 10.0f;
	view = cam->update(eye, pos);
	frustum.setCamDef(eye, pos, glm::vec3(camUp.x, camUp.y, camUp.z));


	models[0]->position.x = characterController->getFootPosition().x;
	models[0]->position.y = characterController->getFootPosition().y;
	models[0]->position.z = characterController->getFootPosition().z;

	models[0]->center.x = characterController->getPosition().x;
	models[0]->center.y = characterController->getPosition().y;
	models[0]->center.z = characterController->getPosition().z;

	

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glViewport(0, 0, width, height);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	
	GLuint depthMapFBO;
	const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC,GL_GREATER);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	float simTimer = 0.0f;
	auto time = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{

		auto time_new = glfwGetTime();
		time_delta = (float)(time_new - time);
		
		time = time_new;

		update(time_delta);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ShadowMap
		glm::mat4 lightView;
		glm::mat4 lightSpaceMatrix;

		
		lightProjection = glm::ortho(-20.0f, 20.0f, -15.0f, 20.0f,  -20.0f,  20.0f);
		lightView = glm::lookAt(lightPos + models[0]->position, models[0]->center, glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		
		// shadowMapRender
		shadowMapShader->useShader();
		glUniformMatrix4fv(glGetUniformLocation(shader->programHandle, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(4.f, 0.f);
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderShadowMap();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	
		// Normal Renderpass
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		shader->useShader();
		glUniformMatrix4fv(glGetUniformLocation(shader->programHandle, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		draw();
	
	
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		double xpos;
		double ypos;

		glfwGetCursorPos(window, &xpos, &ypos);
	
		if (xpos != lastX || ypos != lastY)
		{
			mouseMovementPoll(window, xpos, ypos);
		}

		keyboardInput(window);
	}

	cleanup();

	glfwTerminate();

	return EXIT_SUCCESS;
}


void initPhysX(){
	
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
		gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale());

	//Creating scene
	physx::PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());

	sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

	gScene = gPhysicsSDK->createScene(sceneDesc);
	manager = PxCreateControllerManager(*gScene);
	
	if (gPhysicsSDK == NULL){
		std::cerr << "Error creating PhysX3 device, Exiting" << std::endl;
	}

#ifdef _DEBUG
	physx::PxVisualDebuggerConnectionManager* pvd = gPhysicsSDK->getPvdConnectionManager();
	if (!pvd)
		return;
	physx::PxVisualDebuggerConnectionFlags theConnectionFlags(physx::PxVisualDebuggerConnectionFlag::eDEBUG |
	physx::PxVisualDebuggerConnectionFlag::ePROFILE | physx::PxVisualDebuggerConnectionFlag::eMEMORY);
	physx::PxVisualDebuggerExt::createConnection(pvd, "localhost", 5425, 10000, theConnectionFlags);

	gPhysicsSDK->getVisualDebugger()->setVisualizeConstraints(true);
#endif
	//Creating material
	//static friction, dynamic friction, restitution
	physx::PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5, 0.5, 0.0);

	//2) boundaries 
	physx::PxReal density = 1.0f;
	boundaries.push_back(0);
	for (int i = 1; i < models.size(); i++)
	{
	
			Model *model = models[i].get();
			physx::PxTransform transform(physx::PxVec3(models[i]->position.x, -(glm::abs(model->minVector.y) + glm::abs(model->maxVector.y)) + models[i]->position.y, models[i]->position.z), physx::PxQuat::createIdentity());
			physx::PxVec3 dimensions((1.0f / 2.0f * (glm::abs(model->minVector.x) + glm::abs(model->maxVector.x))), glm::abs(model->minVector.y) + glm::abs(model->maxVector.y), 1.0f / 2.0f *((glm::abs(model->minVector.z) + glm::abs(model->maxVector.z))));
			physx::PxBoxGeometry geometry(dimensions);
			physx::PxRigidStatic* cube = gPhysicsSDK->createRigidStatic(transform);
			cube->createShape(geometry, *mMaterial);
			gScene->addActor(*cube);
			boundaries.push_back(cube);		
	}

	float playerWidth = abs(models[0]->maxVector.x) + abs(models[0]->minVector.x);
	float playerBreadth = abs(models[0]->maxVector.z) + abs(models[0]->minVector.z);
	float playerHeight = abs(models[0]->maxVector.y) +abs(models[0]->minVector.y);

	//PlayerControllerDescription
	characterControllerDesc.height = 1;
	characterControllerDesc.radius = 0.5f;
	characterControllerDesc.slopeLimit = 0.0f;
	characterControllerDesc.stepOffset = 0.00f;
	characterControllerDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
	characterControllerDesc.position = physx::PxExtendedVec3(models[0]->position.x, models[0]->position.y, 	models[0]->position.z);
	characterControllerDesc.contactOffset = 0.1f;
	characterControllerDesc.upDirection = physx::PxVec3(0, 1, 0);
	characterControllerDesc.material = mMaterial;

	
	characterController = manager->createController(characterControllerDesc);
	if (characterController == nullptr){
		std::cout << "CharacterController couldn't be created" << std::endl;
	}
}

void init(GLFWwindow* window)
{

	shader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/toon2.frag");
	shadowMapShader = std::make_unique<Shader>("../Shader/shadowMapShader.vert",
		"../Shader/shadowMapShader.fragment");
	skyBoxShader = std::make_unique<Shader>("../Shader/skyBox.vert",
		"../Shader/skyBox.frag");
	gouradShader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/toon2.frag");

	//cube = std::make_unique<Cube>(glm::mat4(1.0f), shader.get());
	cam = std::make_unique<Camera>(0.0f, 0.0f, 0.0f);
	player = std::make_unique<Model>("../Models/player2.dae", &textures);
	platform1_3 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform0_3 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform2_3 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform0_2 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform1_2 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform2_2 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform0_1 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform1_1 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform2_1 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform0_0 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform1_0 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform2_0 = std::make_unique<Model>("../Models/platform.dae", &textures);
	island = std::make_unique<Model>("../Models/islandSmoothed.dae", &textures);
	plant = std::make_unique<Model>("../Models/plant.dae", &textures);
	platform2 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform3 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform4 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform5 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform6 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform7 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform8 = std::make_unique<Model>("../Models/platform.dae", &textures);
	platform9 = std::make_unique<Model>("../Models/platform.dae", &textures);
	island2 = std::make_unique<Model>("../Models/islandSmoothed.dae", &textures);
	island3 = std::make_unique<Model>("../Models/islandSmoothed.dae", &textures);
	island4 = std::make_unique<Model>("../Models/islandSmoothed.dae", &textures);
	skyBox = std::make_unique<Model>("../Models/skyBox.dae", &textures);
	goal = std::make_unique<Model>("../Models/platform.dae", &textures);


	//Borders

	player->position = glm::vec3(0, 1.1f, 0);
	view = cam->setUp(player->center);
	projection = glm::perspective(glm::radians(60.0f), width / height, near_plane, far_plane);
	frustum.setCamInternals(60.0f, width / height, near_plane, far_plane);
	player->viewMatrix = view;

	parSys = std::make_unique<ParticleSystem>();
	parSys->initialize(player->center);

	contour = std::make_unique<Contour>();
	contour->initialize(width, height);
	
	glm::vec3 islandPos(0.0f, 0.0f, 0.0f);
	island->position = islandPos;
	island->viewMatrix = view;

	platform2->position = glm::vec3(-25.0f, 0.0f, 0.0f) ;
	platform2->viewMatrix = view;

	platform3->position = glm::vec3(-5.0f, 0.0f, 0.0f);
	platform3->viewMatrix = view;
	
	platform1_3->position = glm::vec3(10.0f, 0.0f, 0.0f);
	platform1_3->viewMatrix = view;

	platform0_3->position = glm::vec3(10.0f, 0.0f, -5.0f);
	platform0_3->viewMatrix = view;

	platform2_3->position = glm::vec3(10.0f, 0.0f, 5.0f);
	platform2_3->viewMatrix = view;

	platform1_2->position = glm::vec3(17.0f, 0.0f, 0.0f);
	platform1_2->viewMatrix = view;

	platform0_2->position = glm::vec3(17.0f, 0.0f, -5.0f);
	platform0_2->viewMatrix = view;

	platform2_2->position = glm::vec3(17.0f, 0.0f, 5.0f);
	platform2_2->viewMatrix = view;

	platform1_1->position = glm::vec3(24.0f, 0.0f, 0.0f);
	platform1_1->viewMatrix = view;

	platform0_1->position = glm::vec3(24.0f, 0.0f, -5.0f);
	platform0_1->viewMatrix = view;

	platform2_1->position = glm::vec3(24.0f, 0.0f, 5.0f);
	platform2_1->viewMatrix = view;

	platform1_0->position = glm::vec3(31.0f, 0.0f, 0.0f);
	platform1_0->viewMatrix = view;

	platform0_0->position = glm::vec3(31.0f, 0.0f, -5.0f);
	platform0_0->viewMatrix = view;

	platform2_0->position = glm::vec3(31.0f, 0.0f, 5.0f);
	platform2_0->viewMatrix = view;

	goal->position = glm::vec3(78.0f, 26.0f, 30.0f);
	
	goal->viewMatrix = view;

	/*std::vector<std::vector<int>> sol = calcPath();
	int xCount = 0;
	int yCount = 0;
	for (int i : sol[0]) {
		std::cout << i << std::endl;
		xCount++;
	}

	for (int i : sol[1]) {
		std::cout << i << std::endl;
		yCount++;
	}
	std::cout << "xCount: " << xCount << std::endl;
	std::cout << "yCount: " << yCount << std::endl;*/

	std::vector<int> path1x = { 1, 2, 2, 2, 1, 0, 0 };
	std::vector<int> path1y = { 3, 3, 2, 1, 1, 1, 0 };
	std::vector<int> path1;
	for (int i = 0; i < path1x.size(); i++) 
	{
		path1.push_back(path1x[i]);
		path1.push_back(path1y[i]);
	}

	std::vector<int> path2x = { 1, 1, 0, 0, 0 };
	std::vector<int> path2y = { 3, 2, 2, 1, 0 };
	std::vector<int> path2;
	for (int i = 0; i < path2x.size(); i++)
	{
		path2.push_back(path2x[i]);
		path2.push_back(path2y[i]);
	}

	std::vector<int> path3x = { 1, 2, 2, 2, 2, 1, 0 };
	std::vector<int> path3y = { 3, 3, 2, 1, 0, 0, 0 };
	std::vector<int> path3;
	for (int i = 0; i < path3x.size(); i++)
	{
		path3.push_back(path3x[i]);
		path3.push_back(path3y[i]);
	}

	std::vector<int> path4x = { 1, 0, 0, 0, 0 };
	std::vector<int> path4y = { 3, 3, 2, 1, 0 };
	std::vector<int> path4;
	for (int i = 0; i < path4x.size(); i++)
	{
		path4.push_back(path4x[i]);
		path4.push_back(path4y[i]);
	}

	paths.push_back(path1);
	paths.push_back(path2);
	paths.push_back(path3);
	paths.push_back(path4);

	srand(time(0));
	curPath = paths[rand() % 4];
	
	for (int i = 0; i < curPath.size() - 1; i += 2)
	{
		marked[curPath[i]][curPath[i+1]] = true;
		/*std::cout << curPath[i] << std::endl;
		std::cout << curPath[i+1] << std::endl;*/
	}

	plant->position = glm::vec3(-3.0f, 2*(glm::abs(plant->maxVector.y) + glm::abs(plant->minVector.y)) , 0.0);

	platform2->child = platform3.get();
	
	plant->position = glm::vec3(-3.0f, 2*(glm::abs(plant->maxVector.y) + glm::abs(plant->minVector.y))-0.1f, 0.0);
	plant->viewMatrix = view;


	
	island2->position = glm::vec3(40.0f, 4.0f, -8.0f);
	island2->viewMatrix = view;

	platform4->position = glm::vec3(44.0f, 7.0f, -2.0f);

	island3->position = glm::vec3(55.0f, 11.0f, -4.0f);
	island3->viewMatrix = view;

	platform5->position = glm::vec3(60.0f, 13.0f, -2.0f);
	platform6->position = glm::vec3(63.0f, 17.0f, -4.0f);
	platform7->position = glm::vec3(65.0f, 22.0f,  0.0f);
	platform8->position = glm::vec3(60.0f, 22.0f,  8.0f);
	platform9->position = glm::vec3(65.0f, 22.0f,  16.0f);
	island4->position = glm::vec3(70.0f, 22.0f, 26.0f);
	
	
	platform3->draw();

	skyBox->position = glm::vec3(0, 0, 0);
	skyBox->viewMatrix = view;

	
	//Basic Shader
	shader->useShader();

	int modelView = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));

    int modelProjection = glGetUniformLocation(shader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));
	singleColorLoc = glGetUniformLocation(shader->programHandle, "singleColor");
		
	//Setting MaterialProperties

	int matAmbientPos = glGetUniformLocation(shader->programHandle, "mat.ambient");
	int matDiffusePos = glGetUniformLocation(shader->programHandle, "mat.diffuse");
	int matSpecularPos = glGetUniformLocation(shader->programHandle, "mat.specular");
	int	matShinePos = glGetUniformLocation(shader->programHandle, "mat.shininess");

	glUniform3f(matAmbientPos, 0.4f, 0.4f, 0.4f);
	glUniform3f(matDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(matSpecularPos,1.0f, 1.0f, 1.0f);
	glUniform1f(matShinePos, 10.0f);


	//Setting LightProperties

	int lightAmbientPos = glGetUniformLocation(shader->programHandle, "light.ambient");
	int lightDiffusePos = glGetUniformLocation(shader->programHandle, "light.diffuse");
	int lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
    
	int lightDirectionPos = glGetUniformLocation(shader->programHandle, "light.direction");

	glUniform3f(lightDirectionPos, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(lightAmbientPos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightSpecularPos, 0.0f, 0.0f, 0.0f);

	auto transparency = glGetUniformLocation(shader->programHandle, "transparency");
	glUniform1f(transparency,1.0);

	skyBoxShader->useShader();
	modelView = glGetUniformLocation(skyBoxShader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));
	modelProjection = glGetUniformLocation(skyBoxShader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	text = std::make_unique<TextRenderer>(width, height);
	models.push_back(std::move(player));
	models.push_back(std::move(island));
	models.push_back(std::move(platform1_3));
	models.push_back(std::move(plant));
	models.push_back(std::move(platform2));
	models.push_back(std::move(platform0_3));
	models.push_back(std::move(platform2_3));
	models.push_back(std::move(platform1_2));
	models.push_back(std::move(platform0_2));
	models.push_back(std::move(platform2_2));
	models.push_back(std::move(platform1_1));
	models.push_back(std::move(platform0_1));
	models.push_back(std::move(platform2_1));
	models.push_back(std::move(platform1_0));
	models.push_back(std::move(platform0_0));
	models.push_back(std::move(platform2_0));
	models.push_back(std::move(goal));
	models.push_back(std::move(island2));
	models.push_back(std::move(platform4));
	models.push_back(std::move(platform5));
	models.push_back(std::move(platform6));
	models.push_back(std::move(platform7));
	models.push_back(std::move(platform8));
	models.push_back(std::move(platform9));
	models.push_back(std::move(island4));



	models.push_back(std::move(island3));

	platforms[0][0] = std::move(platform0_0);
	platforms[0][1] = std::move(platform0_1);
	platforms[0][2] = std::move(platform0_2);
	platforms[0][3] = std::move(platform0_3);

	platforms[1][0] = std::move(platform1_0);
	platforms[1][1] = std::move(platform1_1);
	platforms[1][2] = std::move(platform1_2);
	platforms[1][3] = std::move(platform1_0);

	platforms[2][0] = std::move(platform2_0);
	platforms[2][1] = std::move(platform2_1);
	platforms[2][2] = std::move(platform2_2);
	platforms[2][3] = std::move(platform2_3);
}


void StepPhysX()
{

		gScene->simulate(myTimeStep);
	
	while (!gScene->fetchResults())
	{

	}
}

void cleanup()
{

	player.reset(nullptr);
	shader.reset(nullptr);
	cam.reset(nullptr);
	characterController->release();
	//sphere.reset(nullptr);
	
}

void print_fps()
{
	if (frametimeOn) {
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		float frametime = time_delta * 1000;
		float fps = 1.0 / time_delta;
		std::string frames = "Frametime: " + std::to_string(frametime) + " ms =~ " + std::to_string(fps) + " fps ";
		text->drawText(frames, 10, height - 25.0f, 0.4f);
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
}

void print_message()
{
	if (messageTimer > 0) {
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		text->drawText(textMessage, 10, height - 50.0f, 0.4f);
		messageTimer -= time_delta;
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
}

void print_youWin(){
	if (winTimer > 0){
		message = "YOU WIN";
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		text->drawText(message, width/2.0f - 100, height / 2.0f - 50, 2.0f);
		winTimer -= time_delta;
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
}


void draw(){


	models[0]->viewMatrix = view;
	for (int i = 1; i < models.size(); i++){
		models[i]->viewMatrix = view;
	}
	glm::mat4x4 mvp = projection * view;
	parSys->compute();

	// Draw all other objects with one color
	glm::vec4 color(clearColor);
	contour->activate();
	shader->useShader();
	
	color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	glUniform4fv(singleColorLoc, 1, glm::value_ptr(color));
	models[0]->draw();
	contour->deactivate();
	
	// Draw all other objects with textures
	color = glm::vec4(0.0);
	glUniform4fv(singleColorLoc, 1, glm::value_ptr(color));
	
	
	shader->useShader();
for (int i = 0; i < models.size(); i++){
	
		if (frustumOn && i != 0 && i != 4 ){
			if (frustum.boxInFrustum(boundaries[i]->getWorldBounds())){
			
				if (i == 3 && transparencyOn){
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				if (i == 3) {
					glDisable(GL_CULL_FACE);
				}
				if (i == 2 || i == 6 || i == 9 || i == 12 || i == 10 || i == 11 || i == 14){
					if (pathTimer < 200)
					{
						color = glm::vec4(0.0, 0.0, 1.0, 1.0);
						glUniform4fv(singleColorLoc, 1, glm::value_ptr(color));
						models[i]->draw();
						
					}	
					else
					{
						models[i]->draw();
						
					}
				}
				else if (i == 16) {
					if (pathTimer < 200)
					{
						color = glm::vec4(1.0, 1.0, 0.0, 1.0);
						glUniform4fv(singleColorLoc, 1, glm::value_ptr(color));
						models[i]->draw();
				
					}
					else
					{
						models[i]->draw();
					
					}
				}
				else
				{
					models[i]->draw();
			
				}
				color = glm::vec4(0.0);
				glUniform4fv(singleColorLoc, 1, glm::value_ptr(color));
				if (i == 3 && transparencyOn){
					glDisable(GL_BLEND);
				}
				if (i == 3) {
					glEnable(GL_CULL_FACE);
				} 
			}
			else{
				
			}
		}
		else{
			if (i == 3 && transparencyOn){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			if (i == 3) {
				glDisable(GL_CULL_FACE);
			}
			models[i]->draw();
			if (i == 4){
				models[4]->updateChild(models[4]->child->outModel);
			}

			if (i == 3 && transparencyOn){
				glDisable(GL_BLEND);
			}
			if (i == 3) {
				glEnable(GL_CULL_FACE);
			}
		}
		
	}
  
	glDepthFunc(GL_LEQUAL);
	skyBox->viewMatrix = glm::mat4(glm::mat3(view));
	skyBoxShader->useShader();
	skyBox->draw();
	glDepthFunc(GL_LESS);

	// Draw particle system
	parSys->draw(mvp);

	// Draw all texts
	print_fps();
	print_message();
	print_youWin();

	// Draw contour
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (wireframeOn){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	contour->draw();
	shader->useShader();

	pathTimer += 100 * time_delta;
}

void update(float deltaTime)
{

	const physx::PxControllerFilters filters(NULL, NULL, NULL);

	timeSim += deltaTime;
	if (timeSim >= myTimeStep){

		if (disp.y != 0){
			disp.x *= 2.0f;
			disp.z *= 2.0f;
		}

		physx::PxControllerCollisionFlags collisionFlags = characterController->move(disp, 0, myTimeStep, filters);
		
			if ((collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN && isJumping)){
				disp.y = 0;
				isJumping = false;
			}
			else if (!(collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN) || isJumping)
			{
				disp.y -= 0.05f;
			}
			else{
				disp.y = 0;
			}
		
			if (characterController)
		StepPhysX();
			

		models[0]->position.x = characterController->getFootPosition().x;
		models[0]->position.y = characterController->getFootPosition().y;
		models[0]->position.z = characterController->getFootPosition().z;

		models[0]->center.x = characterController->getPosition().x;
		models[0]->center.y = characterController->getPosition().y;
		models[0]->center.z = characterController->getPosition().z;

		camPos = models[0]->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center);
		frustum.setCamDef(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center, cam->up);

		disp.x = 0; disp.z = 0;
		timeSim -= myTimeStep;

		
		
		if (characterController->getPosition().y <= -40){
			characterController->setPosition(physx::PxExtendedVec3(0,0,0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
		}
		
		

		if (characterController->getPosition().z < -3.2 && characterController->getPosition().z > -6.9 && 
			characterController->getPosition().x < 12 && characterController->getPosition().x > 7.8 && characterController->getPosition().y < 1.2){
			characterController->setPosition(physx::PxExtendedVec3(0, 0, 0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
		}
		
		if (characterController->getPosition().z < 1.8 && characterController->getPosition().z > -1.9 &&
			characterController->getPosition().x < 19 && characterController->getPosition().x > 14.6 && characterController->getPosition().y < 1.2){
			characterController->setPosition(physx::PxExtendedVec3(0, 0, 0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
		}
		
		
		if (characterController->getPosition().z < -3.2 && characterController->getPosition().z > -6.9 &&
			characterController->getPosition().x < 19 && characterController->getPosition().x > 14.6 && characterController->getPosition().y < 1.2){
			characterController->setPosition(physx::PxExtendedVec3(0, 0, 0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
		}

		if (characterController->getPosition().z < 1.8 && characterController->getPosition().z > -1.9 &&
			characterController->getPosition().x < 33 && characterController->getPosition().x > 28.6 && characterController->getPosition().y < 1.2){
			characterController->setPosition(physx::PxExtendedVec3(0, 0, 0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
		}

		if (characterController->getPosition().z < 6.8 && characterController->getPosition().z > 3.1 &&
			characterController->getPosition().x < 33 && characterController->getPosition().x > 28.6 && characterController->getPosition().y < 1.2){
			characterController->setPosition(physx::PxExtendedVec3(0, 0, 0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
		}

		//goal->position = glm::vec3(72.0f, 26.0f, 30.0f);
		
		//std::cout << characterController->getPosition().x << " " << characterController->getPosition().z << " " << characterController->getPosition().y << std::endl;
		if (characterController->getPosition().x < 79.90f && characterController->getPosition().x > 70.8 &&
			characterController->getPosition().z < 31.4 && characterController->getPosition().z > 28.4 &&
			characterController->getPosition().y < 27.2 && characterController->getPosition().y > 27.0) {
			
			
			winTimer = 2.0f;

			float winTimer = 0;
			Sleep(100);
			/*while (winTimer < 5.0) {
				winTimer += time_delta;
				//std::cout << "timer" << winTimer << std::endl;
			}*/

			characterController->setPosition(physx::PxExtendedVec3(0, 0, 0));
			models[0]->position = glm::vec3(0, 0, 0);
			pathTimer = 0;
			accepted = false;
		}
		

		/*std::cout << "x-Wert: " << characterController->getPosition().x << std::endl;
		std::cout << "z-Wert: " << characterController->getPosition().z << std::endl;*/
		

	}

	rad += (glm::pi<float>() / 180.0f) * 60 * deltaTime;
	models[4]->position = glm::vec3(models[4]->position.x, models[4]->position.y, sin(rad) * 10.0f);

	models[4]->child->outModel = glm::rotate(glm::mat4(), glm::radians(45.0f)*deltaTime, glm::vec3(0, 1, 0)) * models[4]->child->outModel;

	
}

bool firstMouse = true;
void mouseMovementPoll(GLFWwindow* window, double xpos, double ypos)
{
	
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05;
	GLfloat ySensitivity = 0.0005;
	xoffset *= sensitivity;
	yoffset *= ySensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 29.0f)
		pitch = 29.0f;
	if (pitch < -29.0f)
		pitch = -29.0f;

	auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
	auto t2 = glm::rotate(glm::mat4(), glm::radians(pitch), glm::vec3(1, 0, 0));
	auto t3 = glm::translate(glm::mat4(), models[0]->center);
	auto t4 = t3 * t2 * t1;

	camDirection = (t2 * t1 * glm::vec4(camInitial, 1)).xyz;

	camUp = t2 * t1 * glm::vec4(0, 1, 0, 1);
	/*glm::vec4 eye = t4 * glm::vec4(0.0f, 5.0f, 8.0f, 1.0f);
	cam->eyeX = eye.x;
	cam->eyeY = eye.y;
	cam->eyeZ = eye.z;*/
	glm::vec3 eye = models[0]->center - camDirection * 10.0f;
	cam->eyeX = eye.x;
	cam->eyeY = eye.y;
	cam->eyeZ = eye.z;
	view = cam->useUp(eye.xyz(), models[0]->center, camUp.xyz());
	frustum.setCamDef(eye.xyz(), models[0]->center, camUp.xyz());
	
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
		if (!isJumping  && disp.y >= -0.1 && disp.y <= 0.1){
			disp.y = 0.7;
			isJumping = true;
		}
	}
	if (key == GLFW_KEY_F8 && action == GLFW_PRESS){
		frustumOn = !frustumOn;
		if (frustumOn) {
			textMessage = "Frustum-Culling: On";
			messageTimer = 2.0f;
		}
		else {
			textMessage = "Frustum-Culling: Off";
			messageTimer = 2.0f;
		}
	}
	if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
		frametimeOn = !frametimeOn;
	}
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
		wireframeOn = !wireframeOn;
		if (wireframeOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_CULL_FACE);
			textMessage = "Wireframe: On";
			messageTimer = 2.0f;
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_CULL_FACE);
			textMessage = "Wireframe: Off";
			messageTimer = 2.0f;
		}
	}
	if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
		texfilterOn = !texfilterOn;
		if (texfilterOn) {
			textMessage = "Texture-Filtering: Bilinear";
			messageTimer = 2.0f;

			for (int i = 0; i < textures.size(); i++) {
				glBindTexture(GL_TEXTURE_2D, i);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}
		else {
			textMessage = "Texture-Filtering: Nearest Neighbour";
			messageTimer = 2.0f;

			for (int i = 0; i < textures.size(); i++) {
				glBindTexture(GL_TEXTURE_2D, i);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}
	}
	if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
		mipmapsOn = (mipmapsOn + 1) % 3;
		if (mipmapsOn == 0) {
			textMessage = "Mipmaps: Linear";
			messageTimer = 2.0f;

			for (int i = 0; i < textures.size(); i++) {
				glBindTexture(GL_TEXTURE_2D, i);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
		}
		else if (mipmapsOn == 1) {
			textMessage = "Mipmaps: Nearest Neighbour";
			messageTimer = 2.0f;

			for (int i = 0; i < textures.size(); i++) {
				glBindTexture(GL_TEXTURE_2D, i);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			}
		}
		else {
			textMessage = "Mipmaps: Off";
			messageTimer = 2.0f;

			for (int i = 0; i < textures.size(); i++) {
				glBindTexture(GL_TEXTURE_2D, i);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}
	}
	if (key == GLFW_KEY_F9 && action == GLFW_PRESS) {
		transparencyOn = !transparencyOn;
		if (transparencyOn) {
			textMessage = "Transparency: On";
			messageTimer = 2.0f;
			shader->useShader();
			glUniform1f(7, 1.0);
		}
		else {
			textMessage = "Transparency: Off";
			messageTimer = 2.0f;
			shader->useShader();
			glUniform1f(7, 0.0);
		}
	}
	if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {

		if (gourardOn){
			textMessage = "Specularity: Off";
			messageTimer = 2.0f;
			shader->useShader();
			int lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
			glUniform3f(lightSpecularPos, 0.0f, 0.0f, 0.0f);
			gourardOn = false;
		}
		else{
			textMessage = "Specularity: On";
			messageTimer = 2.0f;
			shader->useShader();
			int lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
			glUniform3f(lightSpecularPos, 1.0f, 1.0f, 1.0f);
			gourardOn = true;
		}
	}
}


void keyboardInput(GLFWwindow* window){
	

	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_F1))
	{
		glTranslatef(0.0f, 0.0f, 0.0f);
	}

	if (glfwGetKey(window, GLFW_KEY_W))
	{

		glm::vec4 oldDirection = glm::vec4(0.0, 0.0, -1.0, 1.0);
		auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
		glm::vec4 newDirection = t1 * oldDirection;

		disp.x += 5.0f*newDirection.x * time_delta;
		disp.z += 5.0f*newDirection.z * time_delta;


		models[0]->position.x = characterController->getFootPosition().x;
		models[0]->position.y = characterController->getFootPosition().y;
		models[0]->position.z = characterController->getFootPosition().z;

		models[0]->center.x = characterController->getPosition().x;
		models[0]->center.y = characterController->getPosition().y;
		models[0]->center.z = characterController->getPosition().z;

		models[0]->angle = glm::radians(-yaw);

		camPos = models[0]->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center);
		frustum.setCamDef(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center, cam->up);

	}
	else if (glfwGetKey(window, GLFW_KEY_S))
	{
		glm::vec4 oldDirection = glm::vec4(0.0, 0.0, 1.0, 1.0);
		auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
		glm::vec4 newDirection = t1 * oldDirection;

		disp.x += 5.0f*newDirection.x * time_delta;
		disp.z += 5.0f*newDirection.z * time_delta;

		models[0]->position.x = characterController->getFootPosition().x;
		models[0]->position.y = characterController->getFootPosition().y;
		models[0]->position.z = characterController->getFootPosition().z;

		models[0]->center.x = characterController->getPosition().x;
		models[0]->center.y = characterController->getPosition().y;
		models[0]->center.z = characterController->getPosition().z;

		models[0]->angle = glm::radians(-yaw + 180);

		camPos = models[0]->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center);
		frustum.setCamDef(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center, cam->up);
	}

	if (glfwGetKey(window, GLFW_KEY_A)){

		glm::vec4 oldDirection = glm::vec4(-1.0, 0.0, 0.0, 1.0);
		auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
		glm::vec4 newDirection = t1 * oldDirection;
		//glm::vec3 deltaPos = glm::vec3(characterController->getPosition().x,characterController->getPosition().y, characterController->getPosition().z);
		
		disp.x += 5.0f*newDirection.x * time_delta;
		disp.z += 5.0f*newDirection.z * time_delta;

		//deltaPos = glm::abs(deltaPos-glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z));
		models[0]->position.x = characterController->getFootPosition().x;
		models[0]->position.y = characterController->getFootPosition().y;
		models[0]->position.z = characterController->getFootPosition().z;

		models[0]->center.x = characterController->getPosition().x;
		models[0]->center.y = characterController->getPosition().y;
		models[0]->center.z = characterController->getPosition().z;

		models[0]->angle = glm::radians(-yaw + 90);

		camPos = models[0]->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center);
		frustum.setCamDef(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center, cam->up);

		/*player->position.x -= 10 * time_delta;

		player->angle = glm::radians(-yaw + 90);





		player->position.x += newDirection.x * time_delta * 10;
		player->position.z += newDirection.z * time_delta * 10;

		player->center.x += newDirection.x * time_delta * 10;
		player->center.z += newDirection.z * time_delta * 10;
		player->update();

		*/
	}
	else if (glfwGetKey(window, GLFW_KEY_D))
	{
		glm::vec4 oldDirection = glm::vec4(1.0, 0.0, 0.0, 1.0);
		auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
		glm::vec4 newDirection = t1 * oldDirection;

		disp.x += 5.0f*newDirection.x * time_delta;
		disp.z += 5.0f*newDirection.z * time_delta;

		models[0]->position.x = characterController->getFootPosition().x;
		models[0]->position.y = characterController->getFootPosition().y;
		models[0]->position.z = characterController->getFootPosition().z;

		models[0]->center.x = characterController->getPosition().x;
		models[0]->center.y = characterController->getPosition().y;
		models[0]->center.z = characterController->getPosition().z;

		models[0]->angle = glm::radians(-yaw - 90);

		camPos = models[0]->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center);
		frustum.setCamDef(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), models[0]->center, cam->up);	

	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
	{
		disp.x *= 1.1;
		disp.z *= 1.1;
	}


	
}

void renderShadowMap(){
	
	for (int i = 0; i < models.size(); i++){
		if (i == 3){
			if (transparencyOn)
				glUniform1f(8, 1.0f);
		}
		models[i]->draw();
		glUniform1f(8, 0.0f);
	}
}

std::vector<std::vector<int>> calcPath()
{
	bool valid = false;
	int x = 1;
	int y = 3;
	int randX = 0;
	int randY = 0;
	std::vector<std::vector<int>> sol;
	std::vector<int> xValues;
	std::vector<int> yValues;
	

	xValues.push_back(1);
	yValues.push_back(3);

	srand(time(0));
	while (x != 1 || y != 0)
	{
		randX = rand() % 2;
		randY = rand() % 2;

		switch (randX)
		{
		case(0) : randX = -1;
			break;
		case(1) : randX = 1;
			break;
		}

		switch (randY)
		{
		case(0) : randY = -1;
			break;
		case(1) : randY = 1;
			break;
		}

		if (x + randX >= 0 && x + randX <= 2)
		{
			
		}
		else 
		{
			continue;
		}

		if (y + randY >= 0 && y + randY <= 3)
		{
			
			
		}
		else
		{
			continue;
		}
		x = x + randX;
		y = y + randY;
		xValues.push_back(x);
		yValues.push_back(y);
	}
	xValues.push_back(1);
	yValues.push_back(0);

	sol.push_back(xValues);
	sol.push_back(yValues);
	return sol;
}

void onShapeHit(const physx::PxControllerShapeHit& h){
	std::cout << "test" << std::endl;
}


static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
	switch (id) {
	case 131185: {
		// Warning: Video Memory as Source


		return;
	}
	case 131218: {
		// Program/shader state performance warning:
		// Fragment Shader is going to be recompiled because the shader key based on GL state mismatches.

		return;
	}
	case 131222: {
		//First draw error

		return;
	}
	default: {
		break;
	}

			 std::string error = FormatDebugOutput(source, type, id, severity, message);
			 std::cout << error << std::endl;
	}
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


