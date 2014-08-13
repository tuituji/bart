#ifndef _BVH_H
#define _BVH_H

#include <vector>
#include "shape.h"

using namespace std;

typedef vector<Shape*> ShapeVec;



BBox surround(const BBox& b1, const BBox& b2);


class BVH 
{
public:
	BVH(Shape *shape) {
		left = NULL;
		right = NULL;
		mShape = shape;
		bbox = shape->boundingBox();
	}
	BVH(Shape *shape1, Shape *shape2) {
		left  = new BVH(shape1);
		right = new BVH(shape2);
		mShape = NULL;
		bbox = surround(shape1->boundingBox(), shape2->boundingBox());	
	}
	BVH(BVH *bvh1, BVH *bvh2, const BBox& box) {
		left  = bvh1;
		right = bvh2;
		bbox  = box;
		mShape = NULL;
	}
	BVH(ShapeVec& shapes);
	BVH* buildBranch (ShapeVec &shapes, int idx0, int idx1, int axis = 0);
    BBox boundingBox(){ return bbox; }
	bool intersect(const Ray& ray, HitRecord&record);
	BBox bbox;
	BVH *left;
	BVH *right;
	Shape* mShape;
};

#endif
