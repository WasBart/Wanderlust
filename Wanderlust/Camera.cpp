#include "Camera.h"
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


	Camera::Camera(GLfloat _eyeX, GLfloat _eyeY, GLfloat _eyeZ)
	{
		this->eyeX = _eyeX;
		this->eyeY = _eyeY;
		this->eyeZ = _eyeZ;
	}

	glm::mat4 Camera::setUp(glm::vec3 & center)
	{
		return glm::lookAt(glm::vec3(this->eyeX, this-> eyeY, this-> eyeZ), center, glm::vec3(0.0f,1.0f,0.0f));
	}

	glm::mat4 Camera::update(glm::vec3 & eye, glm::vec3 & center)
	{
		return glm::lookAt(eye, center, glm::vec3(0, 1, 0));
	}