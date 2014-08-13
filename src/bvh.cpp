#include "bvh.h"
#include <cassert>

int binaryPartition(ShapeVec &shapes, int idx0, int idx1,
		const Vector& midPoint, int axis)
{
	BBox bbox;
	float centroid;
	int ret_idx = 0;
	int i = 0;
	float midPointAxis;

	for(ret_idx = idx0, i = idx0; i <= idx1; ++i) {
		bbox = shapes[i]->boundingBox();
		switch(axis) {
		case 0:
			centroid = ((bbox.boxMin()).x + (bbox.boxMax()).x)/2.0f;
			midPointAxis = midPoint.x;
			break;
		case 1:
			centroid = ((bbox.boxMin()).y + (bbox.boxMax()).y)/2.0f;
			midPointAxis = midPoint.y;
			break;
		case 2:
			centroid = ((bbox.boxMin()).z + (bbox.boxMax()).z)/2.0f;
			midPointAxis = midPoint.z;
			break;
		default:
			assert(axis);
			break;		
		}
		
		if(centroid < midPointAxis) {
			if(i != ret_idx) { 
				Shape *tmp = shapes[i];
				shapes[i] = shapes[ret_idx];
				shapes[ret_idx] = tmp;
			}
			++ret_idx;
		}
		
	}
	if(ret_idx == idx0 || ret_idx == idx1 + 1) 
		ret_idx =  (idx0 + idx1 + 1)/2;
	return ret_idx;
}

BVH::BVH(ShapeVec& shapes)
{
	int num = shapes.size();
	assert(num != 0);
	if(num == 1) {
		*this = BVH(shapes[0]);
		return;
	}
	mShape = NULL;
	bbox = shapes[0]->boundingBox();
	for(int i = 0; i < num; ++i) {
		bbox = surround(bbox, shapes[i]->boundingBox());
	}
	Vector mid_val = (bbox.boxMax() + bbox.boxMin()) / 2.0;
	int mid_idx = binaryPartition(shapes, 0, num -1, mid_val, 0);
	
	left = buildBranch(shapes, 0, mid_idx -1 , 1);
	right = buildBranch(shapes, mid_idx, num -1, 1);
	
}

BVH* BVH::buildBranch(ShapeVec& shapes, int idx0, int idx1, int axis)
{
	int num = idx1 - idx0;
	if(num == 0) return new BVH(shapes[idx0]);
	BBox box = shapes[idx0]->boundingBox();
	for(int i = idx0; i <= idx1; ++i ) {
		box = surround(box, shapes[i]->boundingBox());
	}
	Vector midPoint = (box.boxMin() + box.boxMax())/2.0f;
	int mid_idx = binaryPartition(shapes, idx0, idx1, midPoint, axis);
	BVH *left0  = buildBranch(shapes, idx0, mid_idx-1, (axis+1)%3);
	BVH *right0 = buildBranch(shapes, mid_idx, idx1, (axis+1)%3);
	return new BVH(left0, right0, box);
}


bool BVH::intersect(const Ray& ray, HitRecord &record)
{
	if(!bbox.intersect(ray))  return false;
	if(mShape)                return mShape->intersect(ray, record);

	bool hit1;
	bool hit2;
	HitRecord rec1;
	HitRecord rec2;

	if(left)
		hit1 = left->intersect(ray, rec1);
	if(right)
		hit2 = right->intersect(ray, rec2);
	record = (rec1.t < rec2.t) ? rec1 : rec2;
	return (hit1 || hit2);
}
