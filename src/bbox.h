
#ifndef _BBOX_H
#define _BBOX_H

#include "geometry.h"
#include "ray.h"

class BBox { 
public:
    BBox(const Vector& a, const Vector& b) {
        bb[0] = a;
        bb[1] = b; 
    };
    BBox() {  };
    Vector boxMin() const { return bb[0]; } 
    Vector boxMax() const { return bb[1]; }
    bool intersect(const Ray& r) const;  
private:
    Vector bb[2];
};

BBox surround(const BBox& b1, const BBox& b2);

#endif
