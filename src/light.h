#ifndef _LIGHT_H
#define _LIGHT_H

#include "material.h"
#include "geometry.h"

enum Type {AmbientLIGHT, PointLIGHT,  DirectionalLIGHT };

class Light {
public:
	Light(Type t,const Color& c): type(t), col(c) { };
	virtual Vector getDir(const Point &p, float &t) { 
		return Vector(0.0, 0.0, 0.0); 
		t = INF;
	};
	Type type;
	Color col;
};

class PointLight: public Light {
public:
	PointLight(Point p, Color c) : Light(PointLIGHT, c), pos(p) {  };
	
	inline Vector getDir(const Point& p, float &t) {
		Vector v = Vector(p - pos);
		t = v.Length(); 
		v.normalize();
		return v;
	}

	Point pos;
};

class DirectionalLight : public Light {
public:
	DirectionalLight(Vector v, Color c): Light(DirectionalLIGHT, c), dir(v) {
		dir.normalize();
	};
	
	inline Vector getDir(const Point& p, float &t) {
		t = INF;
		return dir;
	}

	Vector dir;
};

class AmbientLight : public Light {
public:
	AmbientLight(Color col): Light(AmbientLIGHT, col) { };
};

#endif
