#include "geometry.h"

void ONB::initFromUV(const Vector& u, const Vector& v)
{
	U = u;
	U.normalize();
	W = cross(U, v);
	W.normalize();
	V = cross(W, U); // W and U are unit vector,
//	V.normalize();
}

void ONB::initFromWV(const Vector& w, const Vector& v)
{
	W = w;
	W.normalize();
	U = cross(v, W);
	U.normalize();
	V = cross(W, U);
}

