
#include <glm\glm.hpp>
#include "Plane.h"


Plane::Plane(glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3) {

	set3Points(v1, v2, v3);
}


Plane::Plane() {}

Plane::~Plane() {}


void Plane::set3Points(glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3) {


	glm::vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = glm::cross(aux2,aux1);

	normal = glm::normalize(normal);
	point = glm::vec3(v2);
	d = -(normal.x * point.x + normal.y * point.y + normal.z * point.z);
}

void Plane::setNormalAndPoint(glm::vec3 &normal, glm::vec3 &point) {

	this->normal = glm::vec3(normal);
	this->normal = glm::normalize(normal);
	d = -(this->normal.x * point.x + this->normal.y * point.y + this->normal.z * point.z);
}

void Plane::setCoefficients(float a, float b, float c, float d) {

	
	normal = glm::vec3(a, b, c);

	float l = normal.length();

	normal = normal / l;

	this->d = d / l;
}




float Plane::distance(glm::vec3 &p) {

	return d + (normal.x * p.x + normal.y * p.y + normal.z * p.z);
}


