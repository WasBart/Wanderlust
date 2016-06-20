#include <iostream>
#include <memory>
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
#pragma comment(lib, "PhysX3Extensions_x86.lib")
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
#include "ParticleSystem.h"
#include "Contour.h"



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
void RenderScene(Shader* shader);


GLuint planeVAO;

std::unique_ptr<Shader> shader;
std::unique_ptr<Shader> shadowMapShader;
std::unique_ptr<Shader> toonShader;
std::unique_ptr<Shader> debugDepthQuad;


std::unique_ptr<Camera> cam;
std::unique_ptr<Model> player;
std::unique_ptr<Model> plattform;
std::unique_ptr<Model> plattform2;
std::unique_ptr<Model> sphere;
std::unique_ptr<Model> path;
std::unique_ptr<ParticleSystem> parSys;
std::unique_ptr<Contour> contour;

glm::mat4 persp;
glm::mat4 view;
glm::mat4 projection;
glm::vec3 camPos;
glm::vec3 direction = glm::vec3(0.0,0.0,-1.0);
glm::mat4 lightProjection; 

const glm::vec3 camInitial = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camDirection = camInitial;
float width;
float height;
float rad = 0.0f;
float time_delta;

GLfloat near_plane = 0.1f, far_plane = 100.0f;
physx::PxReal myTimeStep = 1.0f / 60.0f;
physx::PxVec3 disp;

GLfloat lastX = 400, lastY = 300;
GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLuint depthMap;
glm::vec4 camUp = glm::vec4(0.0f,1.0f,0.0f,1.0f);

// Light source
glm::vec3 lightPos(-2.0f, 8.0f, -2.0f);


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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

	  GLfloat planeVertices[] = {
        // Positions          // Normals         // Texture Coords
        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
        -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,

        25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
        25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f,
        - 25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f
    };

	GLuint planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);



	//Creating scene
	physx::PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());

	sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

	gScene = gPhysicsSDK->createScene(sceneDesc);
	manager = PxCreateControllerManager(*gScene);
	
	//Creating material
	//static friction, dynamic friction, restitution
	physx::PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5, 0.5, 0.5);

	//Creating static plane is floor for now
	physx::PxTransform planePos = physx::PxTransform(physx::PxVec3(0.0f, 0.0f, 0.0f),
		physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));
	physx::PxRigidStatic* plane = gPhysicsSDK->createRigidStatic(planePos);
	plane->createShape(physx::PxPlaneGeometry(), *mMaterial);
	gScene->addActor(*plane);
	
	//2) Create cube  
	physx::PxReal density = 1.0f;
	physx::PxTransform transform(physx::PxVec3(0.0f, 10.0f, 0.0f), physx::PxQuat::createIdentity());
	physx::PxVec3 dimensions(0.5, 0.5, 0.5);
	physx::PxBoxGeometry geometry(dimensions);

	physx::PxRigidDynamic *actor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *mMaterial, density);
	actor->setAngularDamping(0.75);
	actor->setLinearVelocity(physx::PxVec3(0, 0, 0));
	if (!actor)
		std::cerr << "create actor failed!" << std::endl;
	gScene->addActor(*actor);
	physx::PxBounds3 bounds = actor->getWorldBounds();


	//create Player


	int playerWidth = abs(player->maxVector.x - player->minVector.x);
	int playerBreadth = abs(player->maxVector.z - player->minVector.z);
	int playerHeight = abs(player->maxVector.y - player->minVector.y); 

	//PlayerControllerDescription
	characterControllerDesc.height = playerHeight;
	characterControllerDesc.radius = playerWidth / 2.0f;
	characterControllerDesc.slopeLimit = 0.5f;
	characterControllerDesc.stepOffset = 0.01f;
	characterControllerDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
	characterControllerDesc.position = physx::PxExtendedVec3(player->center.x, player->center.y, player->center.z);
	characterControllerDesc.contactOffset = 0.05f;
	characterControllerDesc.upDirection = physx::PxVec3(0, 1, 0);
	characterControllerDesc.material = mMaterial;
	//CameraControllerDescription

	
	characterController = manager->createController(characterControllerDesc);
	if (characterController == nullptr){
		std::cout << "error" << std::endl;
	}


	glm::vec3 pos, eye;
	pos.x = characterController->getPosition().x;
	pos.y = characterController->getPosition().y;
	pos.z = characterController->getPosition().z;
	eye = pos - camDirection * 10.0f;
	view = cam->update(eye, pos);



	player->position.x = characterController->getFootPosition().x;	
	player->position.y = characterController->getFootPosition().y;
	player->position.z = characterController->getFootPosition().z;

	player->center.x = characterController->getPosition().x;
	player->center.y = characterController->getPosition().y;
	player->center.z = characterController->getPosition().z;

	

	glClearColor(0.35f, 0.36f, 0.43f, 0.3f);
	glViewport(0, 0, width, height);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	
	GLuint depthMapFBO;
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_FUNC,GL_GREATER);

	//Points outside are set to given colour, so they don't appear shadowed
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

		simTimer += time_delta;


		if (simTimer >= myTimeStep){
			if (gScene){
				StepPhysX();
			}
			update(time_delta);
			simTimer -= myTimeStep;
		}
		
		
		/*std::cout << "frametime:" << time_delta * 1000 << "ms"
			<< " =~" << 1.0 / time_delta << "fps" << std::endl;*/

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ShadowMap
		glm::mat4 lightView;
		glm::mat4 lightSpaceMatrix;


		lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f,  0.01f,  20.0f);
		lightView = glm::lookAt(lightPos+player->position, player->center , glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		
		// shadowMapRender
		shadowMapShader->useShader();
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
		//glUniform1i(glGetUniformLocation(shader->programHandle, "depthMap"), depthMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		draw();
		
	


		rad += (glm::pi<float>() / 180.0f) * 20 * time_delta;
	    plattform->position = glm::vec3(5, 0, sin(rad) * 10.0f);
		
		
		plattform2->angle = 2*rad;

		
		int pathWidth = abs(path->minVector.x) + abs(path->maxVector.x);
		int pathBreadth = abs(path->minVector.z) + abs(path->maxVector.z);
		
		//draw();

	

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

GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void initPhysX(){
	
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
		gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale());
	
	if (gPhysicsSDK == NULL){
		std::cerr << "Error creating PhysX3 device, Exiting" << std::endl;
	}

	physx::PxVisualDebuggerConnectionManager* pvd = gPhysicsSDK->getPvdConnectionManager();
	if (!pvd)
		return;
	physx::PxVisualDebuggerConnectionFlags theConnectionFlags(physx::PxVisualDebuggerConnectionFlag::eDEBUG |
	physx::PxVisualDebuggerConnectionFlag::ePROFILE | physx::PxVisualDebuggerConnectionFlag::eMEMORY);
	physx::PxVisualDebuggerExt::createConnection(pvd, "localhost", 5425, 10000, theConnectionFlags);

	gPhysicsSDK->getVisualDebugger()->setVisualizeConstraints(true);
}

void init(GLFWwindow* window)
{

	shader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/basic.frag");
	toonShader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/toon2.frag");
	shadowMapShader = std::make_unique<Shader>("../Shader/shadowMapShader.vert",
		"../Shader/shadowMapShader.fragment");
	debugDepthQuad = std::make_unique<Shader>("../Shader/debugDepthQuad.vert", "../Shader/debugDepthQuad.frag");
	


	//cube = std::make_unique<Cube>(glm::mat4(1.0f), shader.get());
	cam = std::make_unique<Camera>(0.0f, 0.0f, 0.0f);
	player = std::make_unique<Model>("../Models/player.dae");
	plattform = std::make_unique<Model>("../Models/plattform.dae");
	plattform2 = std::make_unique<Model>("../Models/plattform.dae");
	//sphere = std::make_unique<Model>("../Models/sphere.dae");
	path = std::make_unique<Model>("../Models/path.dae");

	player->position = glm::vec3(0, 0.5f, 0);
	view = cam->setUp(player->center);

	projection = glm::perspective(glm::radians(60.0f), width / height, near_plane, far_plane);
	player->viewMatrix = view;
	path->viewMatrix = view;
	path->position = glm::vec3(0, -1.0f, 5.0f);
	path->scale = glm::vec3(10.0f, 1.0f, 10.0f);
	
	plattform->position = glm::vec3(5.0f, -1.0f, 0);
	plattform->viewMatrix = view;


	plattform2->position = glm::vec3(-5.0f, 3.0f, 0);
	plattform2->viewMatrix = view;

	parSys = std::make_unique<ParticleSystem>();
	parSys->initialize(player->center);

	contour = std::make_unique<Contour>();
	contour->initialize(width, height);
	

	//ToonShader
	/*toonShader->useShader();

	int modelView = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));

	int modelProjection = glGetUniformLocation(shader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));

	//Setting MaterialProperties

	GLint matAmbientPos = glGetUniformLocation(shader->programHandle, "mat.ambient");
	GLint matDiffusePos = glGetUniformLocation(shader->programHandle, "mat.diffuse");
	GLint matSpecularPos = glGetUniformLocation(shader->programHandle, "mat.specular");
	GLint matShinePos = glGetUniformLocation(shader->programHandle, "mat.shininess");

	glUniform3f(matAmbientPos, 0.1f, 0.1f, 0.1f);
	glUniform3f(matDiffusePos, 0.3f, 0.3f, 0.7f);
	glUniform3f(matSpecularPos, 0.0f, 0.0f, 0.0f);
	glUniform1f(matShinePos, 40.0f);


	//Setting LightPropertiesl

	GLint lightAmbientPos = glGetUniformLocation(shader->programHandle, "light.ambient");
	GLint lightDiffusePos = glGetUniformLocation(shader->programHandle, "light.diffuse");
	GLint lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
	GLint lightDirectionPos = glGetUniformLocation(shader->programHandle, "light.direction");

	glUniform3f(lightDirectionPos, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(lightAmbientPos, 0.5f, 0.5f, 0.5f);
	glUniform3f(lightDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightSpecularPos, 0.0f, 0.0f, 0.0f);
	glm::vec3 spherePos(-5.0f, 5.0f, -10.0f);
	/*sphere->position = spherePos;
	sphere->viewMatrix = view;
	sphere->update();*/
	
	//Basic Shader
	shader->useShader();

	int modelView = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));

    int modelProjection = glGetUniformLocation(shader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));
		
	//Setting MaterialProperties

	int matAmbientPos = glGetUniformLocation(shader->programHandle, "mat.ambient");
	int matDiffusePos = glGetUniformLocation(shader->programHandle, "mat.diffuse");
	int matSpecularPos = glGetUniformLocation(shader->programHandle, "mat.specular");
	int	matShinePos = glGetUniformLocation(shader->programHandle, "mat.shininess");

	glUniform3f(matAmbientPos, 0.4f, 0.4f, 0.4f);
	glUniform3f(matDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(matSpecularPos,1.0f, 1.0f, 1.0f);
	glUniform1f(matShinePos, 40.0f);


	//Setting LightProperties

	int lightAmbientPos = glGetUniformLocation(shader->programHandle, "light.ambient");
	int lightDiffusePos = glGetUniformLocation(shader->programHandle, "light.diffuse");
	int lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
    int lightDirectionPos = glGetUniformLocation(shader->programHandle, "light.direction");

	glUniform3f(lightDirectionPos, lightPos.x, lightPos.y, lightPos.z);
	glUniform3f(lightAmbientPos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightSpecularPos, 0.0f, 0.0f, 0.0f);

	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
void draw(){
	shader->useShader();
	

	player->viewMatrix = view;

	contour->activate();
	player->draw(shader.get());
	contour->deactivate();

	shader->useShader();
	
	/*sphere->viewMatrix = view;
	sphere->draw(toonShader.get());*/

	path->viewMatrix = view;
	path->draw(shader.get());
	
	plattform->viewMatrix = view;
	plattform->draw(shader.get());
	plattform2->viewMatrix = view;
	plattform2->draw(shader.get());

	glm::mat4x4 mvp = projection * view;
	parSys->draw(glm::vec3(0.0, 0.0, 0.0), mvp);

		

}

void update(float time_delta)
{
	const physx::PxControllerFilters filters(NULL, NULL, NULL);

	if (disp.y != 0){
		disp.x *= 1.1f;
		disp.z *= 1.1f;
	}
	physx::PxControllerCollisionFlags collisionFlags = characterController->move(disp * myTimeStep, 0, myTimeStep, filters);
		
	if ((collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)){
		disp.y = 0;
	}
	else{
		disp.y -= 2;
	}
	
	player->position.x = characterController->getFootPosition().x;
	player->position.y = characterController->getFootPosition().y;
	player->position.z = characterController->getFootPosition().z;

	player->center.x = characterController->getPosition().x;
	player->center.y = characterController->getPosition().y;
	player->center.z = characterController->getPosition().z;

	camPos = player->center - camDirection * 10.0f;
	cam->eyeX = camPos.x;
	cam->eyeY = camPos.y;
	cam->eyeZ = camPos.z;

	view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);
	
	disp.x = 0; disp.z = 0;
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
	auto t3 = glm::translate(glm::mat4(), player->center);
	auto t4 = t3 * t2 * t1;

	camDirection = (t2 * t1 * glm::vec4(camInitial, 1)).xyz;

	camUp = t2 * t1 * glm::vec4(0, 1, 0, 1);
	/*glm::vec4 eye = t4 * glm::vec4(0.0f, 5.0f, 8.0f, 1.0f);
	cam->eyeX = eye.x;
	cam->eyeY = eye.y;
	cam->eyeZ = eye.z;*/
	glm::vec3 eye = player->center - camDirection * 10.0f;
	cam->eyeX = eye.x;
	cam->eyeY = eye.y;
	cam->eyeZ = eye.z;
	view = cam->useUp(eye.xyz(), player->center, camUp.xyz());
	GLint model_view = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){

	if (disp.y == 0){
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
			disp.y = 30;
		}
	}
	
}

void keyboardInput(GLFWwindow* window){
	const physx::PxControllerFilters filters(NULL, NULL, NULL);

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

		disp.x += 0.5f*newDirection.x;
		disp.z += 0.5f*newDirection.z;


		player->position.x = characterController->getFootPosition().x;
		player->position.y = characterController->getFootPosition().y;
		player->position.z = characterController->getFootPosition().z;

		player->center.x = characterController->getPosition().x;
		player->center.y = characterController->getPosition().y;
		player->center.z = characterController->getPosition().z;

		player->angle = glm::radians(-yaw);

		camPos = player->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);

	}
	else if (glfwGetKey(window, GLFW_KEY_S))
	{
		glm::vec4 oldDirection = glm::vec4(0.0, 0.0, 1.0, 1.0);
		auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
		glm::vec4 newDirection = t1 * oldDirection;

		disp.x += 0.5f*newDirection.x;
		disp.z += 0.5f*newDirection.z;

		player->position.x = characterController->getFootPosition().x;
		player->position.y = characterController->getFootPosition().y;
		player->position.z = characterController->getFootPosition().z;

		player->center.x = characterController->getPosition().x;
		player->center.y = characterController->getPosition().y;
		player->center.z = characterController->getPosition().z;

		player->angle = glm::radians(-yaw + 180);

		camPos = player->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);
	}

	if (glfwGetKey(window, GLFW_KEY_A)){

		glm::vec4 oldDirection = glm::vec4(-1.0, 0.0, 0.0, 1.0);
		auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
		glm::vec4 newDirection = t1 * oldDirection;
		//glm::vec3 deltaPos = glm::vec3(characterController->getPosition().x,characterController->getPosition().y, characterController->getPosition().z);
		
		disp.x += 0.5f*newDirection.x;
		disp.z += 0.5f*newDirection.z;

		//deltaPos = glm::abs(deltaPos-glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z));
		player->position.x = characterController->getFootPosition().x;
		player->position.y = characterController->getFootPosition().y;
		player->position.z = characterController->getFootPosition().z;

		player->center.x = characterController->getPosition().x;
		player->center.y = characterController->getPosition().y;
		player->center.z = characterController->getPosition().z;

		player->angle = glm::radians(-yaw + 90);

		camPos = player->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);


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

		disp.x += 0.5f*newDirection.x;
		disp.z += 0.5f*newDirection.z;

		player->position.x = characterController->getFootPosition().x;
		player->position.y = characterController->getFootPosition().y;
		player->position.z = characterController->getFootPosition().z;

		player->center.x = characterController->getPosition().x;
		player->center.y = characterController->getPosition().y;
		player->center.z = characterController->getPosition().z;

		player->angle = glm::radians(-yaw - 90);

		camPos = player->center - camDirection * 10.0f;
		cam->eyeX = camPos.x;
		cam->eyeY = camPos.y;
		cam->eyeZ = camPos.z;

		view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);


	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
	{
		disp.x *= 1.1;
		disp.z *= 1.1;
	}


	
}

void renderShadowMap(){
	
	player->viewMatrix = view;
	player->draw(shadowMapShader.get());


	//sphere->draw(shadowMapShader.get());

	path->viewMatrix = view;
	path->draw(shadowMapShader.get());

	plattform->draw(shadowMapShader.get());

	plattform2->draw(shadowMapShader.get());
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


