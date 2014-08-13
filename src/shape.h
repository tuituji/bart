#ifndef _SHAPE_H
#define _SHAPE_H


#include <vector>
#include <cstdio>
#include "material.h"
#include "geometry.h"
#include "shape.h"
#include "ray.h"
#include "bbox.h"

using namespace std;

class HitRecord;

class Shape {
public:
	virtual bool intersect(const Ray&r, HitRecord& record)
	{
		fprintf(stderr, "intersect not implemented yet\n");
		return false;
	}
	virtual Material getMaterial() {
		fprintf(stderr, "getMaterial not implemented yet\n");
	}
	virtual BBox boundingBox(){ return bbox; }

	virtual void updateBoundingBox() {
		fprintf(stderr, "updateBoundingBox not implemented yet\n");
	}
//	{
//		fprintf(stderr, "boundingBox not implemented yet\n");
//	}
#if 0
	virtual void shapeAnimate(double trans[3], double rot[4], double scal[4]) {
		fprintf(stderr, "shapeAnimate not implemented yet\n");
	}
	void setMaterial(const Material &m) {
		material = m;	
	}
	void setMatrix(const Matrix4x4 &m) {
		transform = m;
		transform_inv = m.Inverse();
	}
	Material material;
	Matrix4x4 transform;
	Matrix4x4 transform_inv;
#endif
protected:
	BBox bbox;
};


class HitRecord {
public:
	HitRecord() : pos(), normal(), t(INF), obj(NULL), tex_col(1.0, 1.0, 1.0){ };

//	Color color;
	Point pos;// do we need pos ?? 
	Vector normal;
	float t;
	Shape *obj;
	Color tex_col;
};

class Sphere : public Shape {
public:
	Sphere(const Point& p, float r) : center(p), radius(r) { };
	bool intersect(const Ray& ray, HitRecord& r);
	void shapeAnimate(double trans[3], double rot[4], double scal[4]);

	Point center;
	float radius;
};

class Plane : public Shape {
public:
	Plane(Vector n, float d): normal(n), distance(d) {
		normal.normalize();
		position = normal * distance;
	};
	bool intersect(const Ray& ray, HitRecord& r);

	Vector normal;
	float distance;
private:
	Point position;
};

#endif
