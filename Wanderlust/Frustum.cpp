#include <glew\glew.h>
#include <glm\glm.hpp>
#include "Frustum.h"
#include <PxPhysicsAPI.h> 

#define ANG2RAD 3.14159265358979323846/180.0



Frustum::Frustum() {}

Frustum::~Frustum() {}

void Frustum::setCamInternals(float angle, float ratio, float nearD, float farD) {

	this->ratio = ratio;
	this->angle = angle;
	this->nearD = nearD;
	this->farD = farD;

	tang = (float)tan(angle* ANG2RAD * 0.5);
	nh = nearD * tang;
	nw = nh * ratio;
	fh = farD  * tang;
	fw = fh * ratio;


}


void Frustum::setCamDef(glm::vec3 &p, glm::vec3 &l, glm::vec3 &u) {

	glm::vec3 dir, nc, fc, X, Y, Z;

	Z = p - l;
	Z = glm::normalize(Z);

	X = u * Z;
	X = glm::normalize(X);

	Y = Z * X;

	nc = p - Z * nearD;
	fc = p - Z * farD;

	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;

	pl[TOP].set3Points(ntr, ntl, ftl);
	pl[BOTTOM].set3Points(nbl, nbr, fbr);
	pl[LEFT].set3Points(ntl, nbl, fbl);
	pl[RIGHT].set3Points(nbr, ntr, fbr);
	pl[NEARP].set3Points(ntl, ntr, nbr);
	pl[FARP].set3Points(ftr, ftl, fbl);
}


int Frustum::pointInFrustum(glm::vec3 &p) {

	int result = INSIDE;
	for (int i = 0; i < 6; i++) {

		if (pl[i].distance(p) < 0)
			return OUTSIDE;
	}
	return(result);

}

int Frustum::boxInFrustum(physx::PxBounds3 bounds){
	int result = INSIDE, out, in;

	// for each plane do ...
	for (int i = 0; i < 6; i++) {

		// reset counters for corners in and out
		out = 0; in = 0;
		// for each corner of the box do ...
		// get out of the cycle as soon as a box as corners
		// both inside and out of the frustum
		glm::vec3 bottomLeftNear = glm::vec3(bounds.minimum.x, bounds.minimum.y, bounds.minimum.z);
		glm::vec3 topLeftNear = glm::vec3(bounds.minimum.x, bounds.maximum.y, bounds.minimum.z);
		glm::vec3 bottomRightNear = glm::vec3(bounds.maximum.x, bounds.minimum.y, bounds.minimum.z);
		glm::vec3 topRightNear = glm::vec3(bounds.maximum.x, bounds.maximum.y, bounds.minimum.z);
		glm::vec3 bottomLeftFar = glm::vec3(bounds.minimum.x, bounds.minimum.y, bounds.maximum.z);
		glm::vec3 topLeftFar = glm::vec3(bounds.minimum.x, bounds.maximum.y, bounds.maximum.z);
		glm::vec3 bottomRightFar = glm::vec3(bounds.maximum.x, bounds.minimum.y, bounds.maximum.z);
		glm::vec3 topRightFar = glm::vec3(bounds.maximum.x, bounds.maximum.y, bounds.maximum.z);

			// is the corner outside or inside
			if (pl[i].distance(bottomLeftNear) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(topLeftNear) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(bottomRightNear) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(topRightNear) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(bottomLeftFar) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(topLeftFar) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(bottomRightFar) < 0)
				out++;
			else
				in++;
			if (pl[i].distance(bottomRightNear) < 0)
				out++;
			else
				in++;
		
		//if all corners are out
		if (!in)
			return (OUTSIDE);
		// if some corners are out and others are in
		else if (out)
			result = INTERSECT;
	return(result);
}
}


