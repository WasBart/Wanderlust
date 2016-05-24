#include <glew/glew.h> 


class Camera
{

public:

	Camera(GLdouble _eyeX, GLdouble _eyeY, GLdouble _eyeZ);
	void setUp(GLdouble centerX, GLdouble centerY, GLdouble centerZ);
	void update(GLdouble centerX, GLdouble centerY, GLdouble centerZ);

private:
	
	GLdouble eyeX;
	GLdouble eyeY;
	GLdouble eyeZ;
};