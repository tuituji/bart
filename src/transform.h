#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include <string>
#include <vector>
#include "geometry.h"

using namespace std;

inline Matrix4x4 Translate(float x, float y, float z)
{
	Matrix4x4 m;
	m[0][3] = x;
	m[1][3] = y;
	m[2][3] = z;
    return m;
}

inline Matrix4x4 Scale(float x, float y, float z)
{
	Matrix4x4 m;
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
	return m;
}


inline Matrix4x4 Rotate(float x, float y, float z, float alpha)
{
	Matrix4x4 m;// m is now an identity matrix
//  To set rotate matrix
	
//	float cos_alpha = cos(deg2rad(alpha));
//	float sin_alpha = sin(deg2rad(alpha));

	float cos_alpha = cos(alpha);
	float sin_alpha = sin(alpha);

	if(x != 0.0 && y == 0.0 && z == 0.0) {  // rotate according to x axis
		m.mat[1][1] = cos_alpha;
		m.mat[1][2] = -sin_alpha;
		m.mat[2][1] = sin_alpha;
		m.mat[2][2] = cos_alpha;
	}
	else if(x == 0.0 && y != 0.0 && z == 0.0) { // rotate according to y axis
		m.mat[0][0]	= cos_alpha;
		m.mat[0][2] = sin_alpha;
		m.mat[2][0] = -sin_alpha;
		m.mat[2][2] = cos_alpha;
	}
	else if(x == 0.0 && y == 0.0 && z != 0.0) {
		m.mat[0][0] = cos_alpha;
		m.mat[0][1] = -sin_alpha;
		m.mat[1][0] = sin_alpha;
		m.mat[1][1] = cos_alpha;
	}
	else {
		Vector axis = Vector(x, y, z)	;
		axis.normalize();
		x = axis.x;
		y = axis.y;
		z = axis.z;
	
		m.mat[0][0] = (1 - cos_alpha) * x * x + cos_alpha;
		m.mat[0][1] = (1 - cos_alpha) * x * y - sin_alpha * y;
		m.mat[0][2] = (1 - cos_alpha) * x * z + sin_alpha * y;
		
		m.mat[1][0] = (1 - cos_alpha) * x * y + sin_alpha * z;
		m.mat[1][1] = (1 - cos_alpha) * y * y + cos_alpha;
		m.mat[1][2] = (1 - cos_alpha) * y * z - sin_alpha * x;

		m.mat[2][0] = (1 - cos_alpha) * x * z - sin_alpha * y;
		m.mat[2][1] = (1 - cos_alpha) * y * z + sin_alpha * x;
		m.mat[2][2] = (1 - cos_alpha) * z * z + cos_alpha;
	}
	return m;
	//mStack.top() *= m;
}

class Mesh;

class TransformHierarchy {
public:
	TransformHierarchy(bool sta) : isStatic(sta) , xName("") { 
		meshPtr = NULL;
		transParent = NULL;
	};

	void addChild(TransformHierarchy* child){
		transChildren.push_back(child);
		child->transParent = this;
	}
	void xFormAnimate(double trans[3], double rot[4], double scal[4]);
	vector<TransformHierarchy*> transChildren;
	bool isStatic;
	string xName;
	TransformHierarchy* transParent;
	Matrix4x4 transformMatrix;  // transform relative to parents
	Matrix4x4 transformMatrixWorld;
	Mesh *meshPtr;
};


#endif
