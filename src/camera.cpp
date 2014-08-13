#include "camera.h"
#include "ray.h"
#include "transform.h"



void Camera::cameraAnimate(double trans[3], double rot[4], double scal[3])
{
	// in fact we do not use scale for camera
	
	Matrix4x4 m;
//	m *= Translate(trans[0], trans[1], trans[2]);
//  m *= Rotate(rot[0], rot[1], rot[2], rot[3]);
//	m *= Translate(-cam_pos.x, -cam_pos.y, - cam_pos.z);	

	cam_pos.x = trans[0];
	cam_pos.y = trans[1];
	cam_pos.z = trans[2];
#if 0
#if 1 
	cam_dir = m * cam_dir0;
	cam_up  = m * cam_up0; 
	
	cam_up.normalize();
	cam_dir.normalize();

	updateUVW();
#else
	cam_dir = m * cam_dir;
	cam_up  = m * cam_up; 
	
	cam_up.normalize();
	cam_dir.normalize();

//	initUVW();
#endif
#endif

#if 0
	Vector axis = rot[0] * uvw.U + rot[1] * uvw.V + rot[2] * uvw.W;
	m *= Rotate(axis.x, axis.y, axis.z, rot[3]);
	uvw.U  = m * uvw.U;
	uvw.V  = m * uvw.V;
	uvw.W  = m * uvw.W;
#else 	
	Vector axis = rot[0] * cam_U0 + rot[1] * cam_V0 + rot[2] * cam_W0;
	m *= Rotate(axis.x, axis.y, axis.z, rot[3]);
	uvw.U  = m * cam_U0;
	uvw.V  = m * cam_V0;
	uvw.W  = m * cam_W0;
#endif

	center = cam_pos;
	uvw.U.normalize();	
	uvw.V.normalize();	
	uvw.W.normalize();
	
	updateUVWParam();

}



// a and b are the pixel positions 
Ray Camera::generateRay(float a, float b)
{
	Point origin = center;
	Vector dir =   a * across + b * up + Vector(corner);
	dir.normalize(); // ok must normalize the ray direction
	return Ray(origin, dir);
}
