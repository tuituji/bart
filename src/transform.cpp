#include "transform.h"


void TransformHierarchy::xFormAnimate(double trans[3], double rot[4], 
		double scal[4])
{
	Matrix4x4 m;
    m *= Translate(trans[0], trans[1], trans[2]);
    m *= Rotate(rot[0], rot[1], rot[2], rot[3]);
    m *= Scale(scal[0], scal[1], scal[2]);

	transformMatrix = m;
}

