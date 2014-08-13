#ifndef _CONE_H
#define _CONE_H

#include "shape.h"

class Cone :public Shape 
{
public:
	bool intersect(const Ray& r, HitRecord& record);
	Material getMaterial();
	void updateBoundingBox();
};
#endif
