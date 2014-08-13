#include "shape.h"

bool Sphere::intersect(const Ray& ray, HitRecord& record)
{
	Vector op = center - ray.pos;
	float eps = 1e-4;
	float b = op * ray.dir;
	float det = b * b - op * op + radius * radius;
	float t = INF;
	if(det < 0) 
		return false;
	else 
		det = sqrt(det);

	t = b - det;
	if(t < eps){ 
		t = b + det;
		if(t < eps) 
			return false;
	}
	record.t = t;
	record.obj = this;
	record.pos = ray.pos + record.t * ray.dir;
	record.normal = record.pos - center;
	record.normal.normalize();
	return true;
}

