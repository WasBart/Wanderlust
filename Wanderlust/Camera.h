#include <glew/glew.h> 
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{

public:

	Camera(GLfloat _eyeX, GLfloat _eyeY, GLfloat _eyeZ);
	glm::mat4 setUp(glm::vec3 & center);
	glm::mat4 update(glm::vec3 & eye, glm::vec3 & center);
	glm::mat4 Camera::useUp(glm::vec3 & eye, glm::vec3 & center, glm::vec3 & up);

	GLfloat eyeX;
	GLfloat eyeY;
	GLfloat eyeZ;

	glm::vec3 up;
private:
	
	
};