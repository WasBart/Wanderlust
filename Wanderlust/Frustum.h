#include <glm\glm.hpp>
#include <PxPhysicsAPI.h>
#include "Plane.h"

class Frustum {
private:

	enum {
		TOP = 0, BOTTOM, LEFT,
		RIGHT, NEARP, FARP

	};

public:

	static enum { OUTSIDE, INTERSECT, INSIDE };
	Plane pl[6];
	glm::vec3 corner[8];
	glm::vec3 ntl, ntr, nbr, nbl, ftl, ftr, fbl, fbr;
	float nearD, farD, ratio, angle, tang;
	float nw, nh, fw, fh;

	Frustum::Frustum();
	Frustum::~Frustum();

	void setCamInternals(float angle, float ratio, float nearD, float farD);
	void setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u);
	int Frustum::pointInFrustum(glm::vec3 &p);
	int Frustum::sphereInFrustum(glm::vec3 &p, float raio);
	
	void Frustum::drawPoints();
	void Frustum::drawLines();
	void Frustum::drawPlanes();
	void Frustum::drawNormals();
	int Frustum::boxInFrustum(physx::PxBounds3 bounds);
};