#include "bbox.h"
#include "ray.h"


BBox surround(const BBox& b1, const BBox& b2) {
    float x1 = b1.boxMin().x;
    float x2 = b2.boxMin().x;
    float x3 = b1.boxMax().x;
    float x4 = b2.boxMax().x;

    float y1 = b1.boxMin().y;
    float y2 = b2.boxMin().y;
    float y3 = b1.boxMax().y;
    float y4 = b2.boxMax().y;

    float z1 = b1.boxMin().z;
    float z2 = b2.boxMin().z;
    float z3 = b1.boxMax().z;
    float z4 = b2.boxMax().z;

    return BBox (
        Vector(x1 < x2 ? x1 : x2, y1 < y2 ? y1 : y2, z1 < z2 ? z1 : z2),
        Vector(x3 > x4 ? x3 : x4, y3 > y4 ? y3 : y4, z3 > z4 ? z3 : z4));
}

bool BBox::intersect(const Ray& r) const 
{
	float t0, t1;
	float min = -INF;
	float max = INF;

	t0 = (bb[r.negPos[0]].x - r.pos.x) * r.dirInv.x;
	t1 = (bb[r.negPos[1]].x - r.pos.x) * r.dirInv.x;
	if (t0 > min) min = t0;
	if (t1 < max) max = t1;
	if (min > max) return false;

	t0 = (bb[r.negPos[2]].y - r.pos.y) * r.dirInv.y;
	t1 = (bb[r.negPos[3]].y - r.pos.y) * r.dirInv.y;
	if (t0 > min) min = t0;
	if (t1 < max) max = t1;
	if (min > max) return false;

	
	t0 = (bb[r.negPos[4]].z - r.pos.z) * r.dirInv.z;
	t1 = (bb[r.negPos[5]].z - r.pos.z) * r.dirInv.z;
	if (t0 > min) min = t0;
	if (t1 < max) max = t1;
	if(min > max) return false;

	return true;
}


