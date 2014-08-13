#ifndef _RAY_H
#define _RAY_H


#include "geometry.h"


class Ray {
public:
    Ray() : pos(), dir(){ } ;
    Ray(const Point &p, const Vector &d): pos(p), dir(d) {
//		dir.normalize();  
		setDir(d);
	}

	void setDir(const Vector &v) {
		dir = v;
		dir.normalize();
		dirInv = Vector(1.0f/dir.x, 1.0/dir.y, 1.0/dir.z);
		negPos[0] = (dir.x > 0 ? 0 : 1);
		negPos[1] = negPos[0] ^ 1;
		negPos[2] = (dir.y > 0 ? 0 : 1); 
		negPos[3] = negPos[2] ^ 1;
		negPos[4] = (dir.z > 0 ? 0 : 1);
		negPos[5] = negPos[4] ^ 1;
	}
    Point pos;
    Vector dir;
	Vector dirInv;
    float t_min, t_mx;
	bool negPos[6];
	
};



#endif
