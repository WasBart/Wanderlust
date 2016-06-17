#include <glew\glew.h>
#include <glm\glm.hpp>
#include "Frustum.h"

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


