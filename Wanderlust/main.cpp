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



using namespace cgue;
using namespace cgue::scene;

static std::string FormatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
static void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

static physx::PxDefaultErrorCallback gDefaultErrorCallback;
static physx::PxDefaultAllocator gDefaultAllocatorCallback;
static physx::PxFoundation* gFoundation = NULL;
static physx::PxPhysics* gPhysicsSDK = NULL;
physx::PxScene* gScene = NULL;
physx::PxReal myTimeStep = 1.0f / 60.0f;
physx::PxControllerManager* manager;
physx::PxCapsuleControllerDesc characterControllerDesc;
physx::PxController* characterController;
physx::PxBoxControllerDesc cameraControllerDesc;
physx::PxController* cameraController;

void init(GLFWwindow* window);
void cleanup();
void draw();
void initPhysX();
void StepPhysX();
void update(float time_delta);
void mouseMovementPoll(GLFWwindow* window, double xpos, double ypos);



std::unique_ptr<Camera> cam;

std::unique_ptr<Shader> shader;
std::unique_ptr<Shader> toonShader;


std::unique_ptr<Model> player;
std::unique_ptr<Model> plattform;
std::unique_ptr<Model> plattform2;
std::unique_ptr<Model> sphere;
std::unique_ptr<Model> path;
std::unique_ptr<Model> path2;
std::unique_ptr<Model> path3;

glm::mat4 persp;
glm::mat4 view;
glm::vec3 cameraPos;
glm::vec3 direction = glm::vec3(0.0,0.0,-1.0);
float width;
float height;
float rad = 0.0f;


GLfloat lastX = 400, lastY = 300;
GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
glm::vec4 camUp = glm::vec4(0.0f,1.0f,0.0f,1.0f);



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

	//Creating static plane
	physx::PxTransform planePos = physx::PxTransform(physx::PxVec3(0.0f, 0.0f, 0.0f),
		physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));
	physx::PxRigidStatic* plane = gPhysicsSDK->createRigidStatic(planePos);
	plane->createShape(physx::PxPlaneGeometry(), *mMaterial);
	gScene->addActor(*plane);

	//2 Create Cube

	physx::PxRigidDynamic*gBox;

	physx::PxTransform boxPos(physx::PxVec3(0.0f, 10000.0f, 0.0f), physx::PxQuat::createIdentity());
	physx::PxBoxGeometry boxGeometry(physx::PxVec3(0.5f, 0.5f, 0.5f));

	gBox = physx::PxCreateDynamic(*gPhysicsSDK, boxPos, boxGeometry, *mMaterial,1.0f);
	gBox->setAngularDamping(0.75);
	gBox->setLinearVelocity(physx::PxVec3(0, 0, 0));
	gScene->addActor(*gBox);
	
	
	//create Player


	int playerWidth = abs(player->maxVector.x) + abs(player->minVector.x);
	int playerBreadth = abs(player->maxVector.z) + abs(player->minVector.z);
	int playerHeight = abs(player->maxVector.y) + abs(player->minVector.y); 

	std::cout << playerWidth << std::endl;
	std::cout << playerBreadth << std::endl;
	std::cout << playerHeight << std::endl;
	//PlayerControllerDescription
	characterControllerDesc.height = playerHeight;
	characterControllerDesc.radius = playerWidth / 2.0f;
	characterControllerDesc.slopeLimit = 0.5f;
	characterControllerDesc.stepOffset = 0.01f;
	characterControllerDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
	characterControllerDesc.position = physx::PxExtendedVec3(0, 0, 0);
	characterControllerDesc.contactOffset = 0.05f;
	characterControllerDesc.upDirection = physx::PxVec3(0, 1, 0);
	characterControllerDesc.material = mMaterial;
	//CameraControllerDescription
	cameraControllerDesc.halfHeight = 0.25f;
	cameraControllerDesc.halfSideExtent = 0.25f;
	cameraControllerDesc.halfForwardExtent = 0.25f;
	cameraControllerDesc.material = mMaterial;
	
	characterController = manager->createController(characterControllerDesc);
	if (characterController == nullptr){
		std::cout << "error" << std::endl;
	}
	cameraController = manager->createController(cameraControllerDesc);
	if (cameraController == nullptr){
		std::cout << "error" << std::endl;
	}

	cameraController->setPosition(physx::PxExtendedVec3(cam->eyeX, cam->eyeY, cam->eyeZ));

	PX_ASSERT(c);
	player->position.x = characterController->getPosition().x;
	player->position.y = characterController->getPosition().y;
	player->position.z = characterController->getPosition().z;

	
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

		
		
			if (gScene){
				StepPhysX();
			}
			physx::PxVec3 p = gBox->getGlobalPose().p;
			//std::cout << "Box current Position (" << boxPos.x << " " << boxPos.y << " " << boxPos.z<< ") " << std::endl;
		

		
		/*std::cout << "frametime:" << time_delta * 1000 << "ms"
			<< " =~" << 1.0 / time_delta << "fps" << std::endl;*/

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		rad += (glm::pi<float>() / 180.0f) * 20 * time_delta;
		plattform->position = glm::vec3(5, 0, sin(rad) * 10.0f);
		
		
		plattform2->angle = 2*rad;

		
		int pathWidth = abs(path->minVector.x) + abs(path->maxVector.x);
		int pathBreadth = abs(path->minVector.z) + abs(path->maxVector.z);

		int msgboxID;
		/*
		if ((player->position.z - playerBreadth / 2.0f <= path->position.z) &&
			player->position.z + playerBreadth / 2.0f >= path->position.z - pathBreadth){

			if (player->position.x + playerWidth / 2.0f  < path->position.x - pathWidth / 2.0f ||
				player->position.x - playerWidth / 2.0f > path->position.x + pathWidth / 2.0f){
				msgboxID = MessageBox(
					NULL,
					L"You lost.\nTry again?",
					L"Don't give up!",
					MB_ICONINFORMATION | MB_YESNO
					);

				if (msgboxID == IDYES)
				{
					init(window);
				}
				else{
					glfwSetWindowShouldClose(window, true);
				}
			}
		}
		else if ((player->position.z - playerBreadth / 2.0f <= path2->position.z) &&
			player->position.z + playerBreadth / 2.0f >= path2->position.z - pathBreadth){

			if (player->position.x + playerWidth / 2.0f< path2->position.x - pathWidth / 8.0f ||
				player->position.x - playerWidth / 2.0f> path2->position.x + pathWidth / 8.0f){
				msgboxID = MessageBox(
					NULL,
					L"You lost.\nTry again?",
					L"Don't give up!",
					MB_ICONINFORMATION | MB_YESNO
					);

				if (msgboxID == IDYES)
				{
					init(window);
				}
				else{
					glfwSetWindowShouldClose(window, true);
				}
			}

		}
		else if ((player->position.z - playerBreadth / 2.0f <= path3->position.z) &&
			player->position.z + playerBreadth / 2.0f >= path3->position.z - pathBreadth / 5.0f){
			msgboxID = MessageBox(
				NULL,
				L"You won!\nHow 'bout another round?",
				L"Congratulations!",
				MB_ICONINFORMATION | MB_YESNO
				);

			if (msgboxID == IDYES)
			{
				init(window);
			}
			else{
			glfwSetWindowShouldClose(window, true);
			}
		}
		else{
			msgboxID = MessageBox(
				NULL,
				L"You lost.\nTry again?",
				L"Don't give up!",
				MB_ICONINFORMATION | MB_YESNO
				);

			if (msgboxID == IDYES)
			{
				init(window);
			}
			else{
				glfwSetWindowShouldClose(window, true);
			}
		}
		*/
		draw();

		const physx::PxControllerFilters filters(NULL, NULL, NULL);

		glfwSwapBuffers(window);
		glfwPollEvents();
		
		double xpos;
		double ypos;

		glfwGetCursorPos(window, &xpos, &ypos);
	
		if (xpos != lastX || ypos != lastY)
		{
			mouseMovementPoll(window, xpos, ypos);
		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, true);
		}
		
		if (glfwGetKey(window, GLFW_KEY_F1))
		{
			glTranslatef(0.0f, 0.0f, 0.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_A)){
			
			glm::vec4 oldDirection = glm::vec4(-1.0, 0.0, 0.0, 1.0);
			auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
			glm::vec4 newDirection = t1 * oldDirection;
			glm::vec3 pos = glm::vec3(characterController->getPosition().x,characterController->getPosition().y, characterController->getPosition().z);
			characterController->move(physx::PxVec3(0.1f*newDirection.x, 0, 0.1f*newDirection.z), 0.0, time_delta, filters);
			//cameraController->move(physx::PxVec3(0.01f*newDirection.x, 0, 0), 0.0, time_delta, filters);
			pos = glm::abs(pos-glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z));
			player->position.x = characterController->getFootPosition().x;
			player->position.z = characterController->getFootPosition().z;
			
			player->center.x = characterController->getPosition().x;
			player->center.y = characterController->getPosition().y;
			player->center.z = characterController->getPosition().z;
		
			player->angle = glm::radians(-yaw + 90);
			
			cam->eyeX += newDirection.x*pos.x;
			cam->eyeY += newDirection.y*pos.y;
			cam->eyeZ += newDirection.z*pos.z;
			//cam->eyeX = player->position.x;

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
			auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
			glm::vec4 oldDirection = glm::vec4(1.0, 0.0, 0.0, 1.0);
			glm::vec4 newDirection = t1 * oldDirection;
			
			player->angle = glm::radians(-yaw - 90);
			
			glm::vec3 pos = glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
			characterController->move(physx::PxVec3(0.1f*newDirection.x, 0, 0.1f*newDirection.z), 0.0, time_delta, filters);
			
			pos = glm::abs(pos - glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z));
			player->position.x = characterController->getFootPosition().x;

			player->center.x = characterController->getPosition().x;
			player->center.y = characterController->getPosition().y;
			player->center.z = characterController->getPosition().z;

			cam->eyeX += newDirection.x*pos.x;
			cam->eyeZ += newDirection.z*pos.z;
			//cam->eyeX = player->position.x;

			view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);

			//player->position.x += 10* time_delta;
			//player->center.x += 10 * time_delta;
			/*player->angle = glm::radians(-yaw - 90);
			auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
			glm::vec4 oldDirection = glm::vec4(1.0, 0.0, 0.0, 1.0);
			glm::vec4 newDirection = t1 * oldDirection;

			player->position.x += newDirection.x * time_delta * 10;
			player->position.z += newDirection.z * time_delta * 10;

			player->center.x += newDirection.x * time_delta * 10;
			player->center.z += newDirection.z * time_delta * 10;
			player->update();

			view = cam->update(glm::vec3(cam->eyeX += newDirection.x * time_delta * 10, cam->eyeY, cam->eyeZ += newDirection.z * time_delta * 10), player->center);*/
	
		}

		if (glfwGetKey(window, GLFW_KEY_W))
		{

			auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
			glm::vec4 oldDirection = glm::vec4(direction.x, direction.y, direction.z, 1.0);
			glm::vec4 newDirection = t1 * oldDirection;

			glm::vec3 pos = glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z);
			characterController->move(physx::PxVec3(0.1f*newDirection.x, 0, 0.1f*newDirection.z), 0.0, time_delta, filters);

			pos = glm::abs(pos - glm::vec3(characterController->getPosition().x, characterController->getPosition().y, characterController->getPosition().z));
			player->position.z = characterController->getFootPosition().z;
			player->position.x = characterController->getFootPosition().x;

			player->center.x = characterController->getPosition().x;
			player->center.y = characterController->getPosition().y;
			player->center.z = characterController->getPosition().z;

			cam->eyeX += newDirection.x*pos.x;
			cam->eyeZ += newDirection.z*pos.z;

			view = cam->update(glm::vec3(cam->eyeX, cam->eyeY, cam->eyeZ), player->center);

			//player->position.z -= 10 * time_delta;
			//player->center.z -= 10 * time_delta;
			/*player->angle = glm::radians(-yaw);
			auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
			glm::vec4 oldDirection = glm::vec4(direction.x, direction.y, direction.z, 1.0);
			glm::vec4 newDirection = t1 * oldDirection;

			
			player->position.x += newDirection.x * time_delta * 10;
			player->position.z += newDirection.z * time_delta * 10;

			player->center.x += newDirection.x * time_delta * 10;
			player->center.z += newDirection.z * time_delta * 10;
			player->update();

			view = cam->update(glm::vec3(cam->eyeX += newDirection.x * time_delta * 10, cam->eyeY, cam->eyeZ += newDirection.z * time_delta * 10), player->center);
			GLint model_view = glGetUniformLocation(shader->programHandle, "view");
			glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));*/
		}
		else if (glfwGetKey(window, GLFW_KEY_S))
		{
			//player->position.z += 10 * time_delta;
			//player->center.z += 10 * time_delta;
			player->angle = glm::radians(-yaw + 180);
			auto t1 = glm::rotate(glm::mat4(), glm::radians(-yaw), glm::vec3(0, 1, 0));
			glm::vec4 oldDirection = glm::vec4(direction.x * -1.0, direction.y * -1.0, direction.z * -1.0, 1.0);
			glm::vec4 newDirection = t1 * oldDirection;

			player->position.x += newDirection.x * time_delta * 10;
			player->position.z += newDirection.z * time_delta * 10;

			player->center.x += newDirection.x * time_delta * 10;
			player->center.z += newDirection.z * time_delta * 10;
			player->update();

			view = cam->update(glm::vec3(cam->eyeX += newDirection.x * time_delta * 10, cam->eyeY, cam->eyeZ += newDirection.z * time_delta * 10), player->center);
			GLint model_view = glGetUniformLocation(shader->programHandle, "view");
			glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
		}
	}

	cleanup();

	glfwTerminate();

	return EXIT_SUCCESS;
}

void initPhysX(){
	
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback,
		gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale());
	
	if (gPhysicsSDK == NULL){
		std::cerr << "Error creating PhysX3 device, Exiting" << std::endl;
	}
}

void init(GLFWwindow* window)
{

	shader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/basic.frag");
	toonShader = std::make_unique<Shader>("../Shader/basic.vert",
		"../Shader/toon2.frag");

	//cube = std::make_unique<Cube>(glm::mat4(1.0f), shader.get());
	cam = std::make_unique<Camera>(0.0f, 5.0f, 8.0f);
	player = std::make_unique<Model>("../Models/player.dae");
	plattform = std::make_unique<Model>("../Models/plattform.dae");
	plattform2 = std::make_unique<Model>("../Models/plattform.dae");
	sphere = std::make_unique<Model>("../Models/sphere.dae");
	path = std::make_unique<Model>("../Models/path.dae");
	path2 = std::make_unique<Model>("../Models/path.dae");
	path3 = std::make_unique<Model>("../Models/path.dae");

	
	glm::mat4 projection;
	player->position = glm::vec3(0, 0, 0);
	player->update();
	//view = cam->setUp(player->center);

	projection = glm::perspective(glm::radians(60.0f), width / height, 0.1f, 100.0f);
	player->viewMatrix = view;
	path->viewMatrix = view;
	path->position = glm::vec3(0, -1.0f, 1.0f);
	path->update();

	path2->position = glm::vec3(-1.0f, -1.0f, path->position.z - abs(path->minVector.z) + abs(path->maxVector.z));
	path2->scale = glm::vec3(0.25f, 1.0f, 1.0f);
	path2->update();

	path3->position = glm::vec3(-0.7f, -1.0f, path2->position.z - abs(path->minVector.z) + abs(path->maxVector.z));
	path3->scale = glm::vec3(1.0f, 1.0f, 0.2f);
	path3->update();

	player->position = glm::vec3(0, 0, 0);
	player->update();
	plattform->position = glm::vec3(5.0f, -1.0f, 0);
	plattform->viewMatrix = view;
	plattform->update();

	plattform2->position = glm::vec3(-5.0f, 3.0f, 0);
	plattform2->viewMatrix = view;
	plattform2->update();

	

	//ToonShader
	toonShader->useShader();

	int modelView = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));

	int modelProjection = glGetUniformLocation(shader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));

	//Setting MaterialProperties

	GLint matAmbientPos = glGetUniformLocation(shader->programHandle, "mat.ambient");
	GLint matDiffusePos = glGetUniformLocation(shader->programHandle, "mat.diffuse");
	GLint matSpecularPos = glGetUniformLocation(shader->programHandle, "mat.specular");
	GLint matShinePos = glGetUniformLocation(shader->programHandle, "mat.shininess");

	glUniform3f(matAmbientPos, 0.5f, 0.5f, 0.5f);
	glUniform3f(matDiffusePos, 0.3f, 0.3f, 0.7f);
	glUniform3f(matSpecularPos, 0.0f, 0.0f, 0.0f);
	glUniform1f(matShinePos, 40.0f);


	//Setting LightProperties

	GLint lightAmbientPos = glGetUniformLocation(shader->programHandle, "light.ambient");
	GLint lightDiffusePos = glGetUniformLocation(shader->programHandle, "light.diffuse");
	GLint lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
	GLint lightDirectionPos = glGetUniformLocation(shader->programHandle, "light.direction");

	glUniform3f(lightDirectionPos, -1.0f, -1.0f, -1.0f);
	glUniform3f(lightAmbientPos, 0.5f, 0.5f, 0.5f);
	glUniform3f(lightDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightSpecularPos, 1.0f, 1.0f, 1.0f);
	glm::vec3 spherePos(-5.0f, 5.0f, -10.0f);
	sphere->position = spherePos;
	sphere->viewMatrix = view;
	sphere->update();

	//Basic Shader
	shader->useShader();

	modelView = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(modelView, 1, GL_FALSE, glm::value_ptr(view));

    modelProjection = glGetUniformLocation(shader->programHandle, "projection");
	glUniformMatrix4fv(modelProjection, 1, GL_FALSE, glm::value_ptr(projection));
		
	//Setting MaterialProperties

	matAmbientPos = glGetUniformLocation(shader->programHandle, "mat.ambient");
	matDiffusePos = glGetUniformLocation(shader->programHandle, "mat.diffuse");
	matSpecularPos = glGetUniformLocation(shader->programHandle, "mat.specular");
	matShinePos = glGetUniformLocation(shader->programHandle, "mat.shininess");

	glUniform3f(matAmbientPos, 0.1f, 0.1f, 0.1f);
	glUniform3f(matDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(matSpecularPos,1.0f, 1.0f, 1.0f);
	glUniform1f(matShinePos, 40.0f);


	//Setting LightProperties

	lightAmbientPos = glGetUniformLocation(shader->programHandle, "light.ambient");
	lightDiffusePos = glGetUniformLocation(shader->programHandle, "light.diffuse");
	lightSpecularPos = glGetUniformLocation(shader->programHandle, "light.specular");
    lightDirectionPos = glGetUniformLocation(shader->programHandle, "light.direction");

	glUniform3f(lightDirectionPos, -1.0f, -1.0f, -1.0f);
	glUniform3f(lightAmbientPos, 0.2f, 0.2f, 0.2f);
	glUniform3f(lightDiffusePos, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightSpecularPos, 1.0f, 1.0f, 1.0f);


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
	sphere.reset(nullptr);
	
}
void draw(){
			

	

	toonShader->useShader();
	player->viewMatrix = view;
	player->draw(shader.get());
	
	sphere->viewMatrix = view;
	sphere->draw(toonShader.get());

	shader->useShader();
	path->viewMatrix = view;
	path->draw(shader.get());
	
	path2->viewMatrix = view;
	path2->draw(shader.get());

	path3->viewMatrix = view;
	path3->draw(shader.get());

	plattform->viewMatrix = view;
	plattform->draw(shader.get());
	plattform2->viewMatrix = view;
	plattform2->draw(shader.get());

	

}

void update(float time_delta)
{
	player->update();
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
	GLfloat ySensitivity = 0.005;
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

	camUp = t2 * t1 * glm::vec4(0, 1, 0, 1);
	glm::vec4 eye = t4 * glm::vec4(0.0f, 5.0f, 8.0f, 1.0f);
	cam->eyeX = eye.x;
	cam->eyeY = eye.y;
	cam->eyeZ = eye.z;
	view = cam->useUp(eye.xyz(), player->center, camUp.xyz());
	GLint model_view = glGetUniformLocation(shader->programHandle, "view");
	glUniformMatrix4fv(model_view, 1, GL_FALSE, glm::value_ptr(view));
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


