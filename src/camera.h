#ifndef _CAMERA_H
#define _CAMERA_H

#include "ray.h"
#include "geometry.h"

class Camera {
public:
//	Camera():cam_pos(), cam_dir(), cam_up(), 
//		cam_fovy(0.0), uvw() { };
	
//	Camera(Vector &p, Vector &d, Vector &u, float f):
//		cam_pos(p), cam_dir(d), cam_up(u), cam_fovy(f) { }; 

	void updateUVW(){
		center = cam_pos;
		uvw.initFromWV(-cam_dir, cam_up);
		updateUVWParam();	
	}
	void initUVW() {
		center = cam_pos;
		uvw.initFromWV(-cam_dir, cam_up);
		cam_U0 = uvw.U;
		cam_V0 = uvw.V;
		cam_W0 = uvw.W;
		updateUVWParam();
	}
	void updateUVWParam() {
		corner = center + uvw.u()*u0 + uvw.v()*v0 - uvw.w() * distance;
		across = uvw.u() * (u1 - u0);  // should be u1 - u0 ??
		up = uvw.v() * (v1 - v0);  // up here should be the same as cam_up ??
	}
	Camera(Point p, Point lookat , Vector u, float f, int width, int height):
		cam_pos(p), cam_dir(lookat - p), cam_up(u), cam_fovy(f), uvw() { 

		cam_dir.normalize();
		cam_up.normalize();

		cam_pos0 = cam_pos;
		cam_dir0 = cam_dir;
		cam_up0  = cam_up;		

		distance = height/ 2 * tanf (deg2rad(90 - cam_fovy/2.0));
	// 2.0 need
		u0 = -width / 2.0;
		v0 = -height/ 2.0;
		u1 =  width / 2.0;
		v1 =  height/ 2.0;

		cam_pos0 = Point(0, 0, 0);
		//cam_up0 = Vector(0, 1, 0);
		//cam_dir0 = Vector(0, 0, 1);
		initUVW();	
//		float t = distance/sqrt(cam_dir.lengthSquared());
//		center = Point(cam_pos + t * cam_dir);
		// the left bootom of the view plane;
	}; 

	void cameraAnimate(double trans[4], double rot[4], double scal[3]);
	Ray generateRay(float a, float b);
	Point cam_pos;
	Vector cam_dir;
	Vector cam_up;
	Point cam_lookat;

	Point cam_pos0;
	Vector cam_dir0;
	Vector cam_up0;

	Vector cam_U0;
	Vector cam_V0;
	Vector cam_W0;

	float cam_fovy;

	ONB uvw;
	Point center; // should be in world coordinate
	Point corner; // should be in world coordinate
	Vector across;
	Vector up;

	float u0, u1, v0, v1;
	float distance;
};

#endif
