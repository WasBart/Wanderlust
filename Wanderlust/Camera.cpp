#include "Camera.h"


	Camera::Camera(GLdouble _eyeX, GLdouble _eyeY, GLdouble _eyeZ)
	{
		this->eyeX = _eyeX;
		this->eyeY = _eyeY;
		this->eyeZ = _eyeZ;
	}

	void Camera::setUp(GLdouble centerX, GLdouble centerY, GLdouble centerZ)
	{
		//gluLookAt(this->eyeX, this->eyeY, this->eyeZ, centerX, centerY, centerZ, 0, 1, 0);
	}

	void Camera::update(GLdouble centerX, GLdouble centerY, GLdouble centerZ)
	{
		this->setUp(centerX, centerY, centerZ);
	}